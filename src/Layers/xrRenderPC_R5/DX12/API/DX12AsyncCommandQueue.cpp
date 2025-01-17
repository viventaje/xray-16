/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#include "StdAfx.h"
#include "DX12AsyncCommandQueue.hpp"
#include "DX12CommandList.hpp"
#include "DX12Device.hpp"

namespace DX12
{
void AsyncCommandQueue::SExecuteCommandlist::Process(const STaskArgs& args)
{
    args.pCommandListPool->GetD3D12CommandQueue()->ExecuteCommandLists(1, &pCommandList);
}

void AsyncCommandQueue::SResetCommandlist::Process(const STaskArgs& args)
{
    pCommandList->Reset();
}

void AsyncCommandQueue::SSignalFence::Process(const STaskArgs& args)
{
    args.pCommandListPool->GetD3D12CommandQueue()->Signal(pFence, FenceValue);
    args.pCommandListPool->SetSignalledFenceValue(FenceValue);
}

void AsyncCommandQueue::SWaitForFence::Process(const STaskArgs& args)
{
    args.pCommandListPool->GetD3D12CommandQueue()->Wait(pFence, FenceValue);
}

void AsyncCommandQueue::SWaitForFences::Process(const STaskArgs& args)
{
    if (FenceValues[CMDQUEUE_COPY])
    {
        args.pCommandListPool->GetD3D12CommandQueue()->Wait(pFences[CMDQUEUE_COPY], FenceValues[CMDQUEUE_COPY]);
    }
    
    if (FenceValues[CMDQUEUE_GRAPHICS])
    {
        args.pCommandListPool->GetD3D12CommandQueue()->Wait(pFences[CMDQUEUE_GRAPHICS], FenceValues[CMDQUEUE_GRAPHICS]);
    }
}

AsyncCommandQueue::AsyncCommandQueue() 
    : m_pCmdListPool(NULL)
    , m_QueuedFramesCounter(0)
    , m_bStopRequested(false) 
{
    m_Semaphore = (void*)CreateSemaphore(NULL, 0, INT_MAX, NULL);
}

AsyncCommandQueue::~AsyncCommandQueue()
{
    SignalStop();
    Flush();

    CloseHandle((HANDLE)m_Semaphore);
}

bool AsyncCommandQueue::IsSynchronous()
{
    return ps_r2_ls_flags_ext.test(R5FLAGEXT_SUBMISSION_THREAD);
}

bool AsyncCommandQueue::Init(CommandListPool* pCommandListPool)
{
    m_pCmdListPool = pCommandListPool;
    m_QueuedFramesCounter = 0;
    m_QueuedTasksCounter = 0;
    m_bStopRequested = false;

    return Threading::SpawnThread(
        [](void* this_ptr) {
            AsyncCommandQueue& self = *static_cast<AsyncCommandQueue*>(this_ptr);
            self.ThreadEntry();
        },
        "DX12 AsyncCommandQueue", 0, this
    );
}

void AsyncCommandQueue::ExecuteCommandLists(UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    for (int i = 0; i < NumCommandLists; ++i)
    {
        SSubmissionTask task;
        ZeroMemory(&task, sizeof(SSubmissionTask));

        task.type = eTT_ExecuteCommandList;
        task.Data.ExecuteCommandList.pCommandList = ppCommandLists[i];

        AddTask<SExecuteCommandlist>(task);
    }
}

void AsyncCommandQueue::ResetCommandList(CommandList* pCommandList)
{
    SSubmissionTask task;
    ZeroMemory(&task, sizeof(SSubmissionTask));

    task.type = eTT_ResetCommandList;
    task.Data.ResetCommandList.pCommandList = pCommandList;

    AddTask<SResetCommandlist>(task);
}

void AsyncCommandQueue::Signal(ID3D12Fence* pFence, const UINT64 FenceValue)
{
    SSubmissionTask task;
    ZeroMemory(&task, sizeof(SSubmissionTask));

    task.type = eTT_SignalFence;
    task.Data.SignalFence.pFence = pFence;
    task.Data.SignalFence.FenceValue = FenceValue;

    AddTask<SSignalFence>(task);
}

void AsyncCommandQueue::Wait(ID3D12Fence* pFence, const UINT64 FenceValue)
{
    SSubmissionTask task;
    ZeroMemory(&task, sizeof(SSubmissionTask));

    task.type = eTT_WaitForFence;
    task.Data.WaitForFence.pFence = pFence;
    task.Data.WaitForFence.FenceValue = FenceValue;

    AddTask<SWaitForFence>(task);
}

void AsyncCommandQueue::Wait(ID3D12Fence** pFences, const UINT64 (&FenceValues)[CMDQUEUE_NUM])
{
    SSubmissionTask task;
    ZeroMemory(&task, sizeof(SSubmissionTask));

    task.type = eTT_WaitForFences;
    task.Data.WaitForFences.pFences = pFences;
    task.Data.WaitForFences.FenceValues[CMDQUEUE_COPY] = FenceValues[CMDQUEUE_COPY];
    task.Data.WaitForFences.FenceValues[CMDQUEUE_GRAPHICS] = FenceValues[CMDQUEUE_GRAPHICS];

    AddTask<SWaitForFences>(task);
}

void AsyncCommandQueue::Flush(UINT64 lowerBoundFenceValue)
{
    if (lowerBoundFenceValue != (~0ULL))
    {
        while (m_QueuedTasksCounter > 0)
        {
            if (lowerBoundFenceValue <= m_pCmdListPool->GetLastCompletedFenceValue())
            {
                break;
            }

            SwitchToThread();
        }
    }
    else
    {
        while (m_QueuedTasksCounter > 0)
        {
            SwitchToThread();
        }
    }
}

void AsyncCommandQueue::ThreadEntry()
{
    while (!m_bStopRequested)
    {
        SSubmissionTask task;

        // if the count would have been 0 or below, go to kernel semaphore
        if (InterlockedDecrement((volatile LONG*)&m_QueuedTasksCounter) < 0)
        {
            WaitForSingleObject((HANDLE)m_Semaphore, INFINITE);
        }

        STaskArgs taskArgs = {m_pCmdListPool, &m_QueuedFramesCounter};

        while (m_TaskQueue.dequeue(task))
        {
            switch (task.type)
            {
            case eTT_ExecuteCommandList: task.Process<SExecuteCommandlist>(taskArgs); break;
            case eTT_ResetCommandList: task.Process<SResetCommandlist>(taskArgs); break;
            case eTT_SignalFence: task.Process<SSignalFence>(taskArgs); break;
            case eTT_WaitForFence: task.Process<SWaitForFence>(taskArgs); break;
            case eTT_WaitForFences: task.Process<SWaitForFences>(taskArgs); break;
            }
        }
    }
}
}
