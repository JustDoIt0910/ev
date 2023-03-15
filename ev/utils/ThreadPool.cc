//
// Created by zr on 23-3-14.
//

#include "ThreadPool.h"

namespace ev
{
    ThreadPool::ThreadPool(int poolSize):
        poolSize_(poolSize),
        running_(false)
    {
        threads_.resize(poolSize_);
    }
}