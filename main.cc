#include <functional>


int main(int argc, char const *argv[])
{
    std::function<void()> cb;
    cb();
    return 0;
}
