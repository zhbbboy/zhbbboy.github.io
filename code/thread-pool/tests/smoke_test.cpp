#include "thread_pool/project.hpp"

int main() {
    return thread_pool::project_name == "thread-pool" ? 0 : 1;
}
