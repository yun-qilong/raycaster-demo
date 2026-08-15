// src/NoHeap.cpp — 无堆红线：Linux 调试期任何 new/delete 立即报错
// 仅链接进 raycaster 可执行目标（测试目标不链接，gtest 需要正常内存管理）。
#include <cstddef>
#include <cstdlib>

void *operator new(std::size_t size)
{
    (void)size;
    std::abort();
}

void *operator new[](std::size_t size)
{
    (void)size;
    std::abort();
}

void operator delete(void *ptr) noexcept
{
    (void)ptr;
}

void operator delete[](void *ptr) noexcept
{
    (void)ptr;
}

void operator delete(void *ptr, std::size_t size) noexcept
{
    (void)ptr;
    (void)size;
}

void operator delete[](void *ptr, std::size_t size) noexcept
{
    (void)ptr;
    (void)size;
}
