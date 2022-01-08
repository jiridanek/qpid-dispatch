extern "C" {
int locally_defined_function() {
    return 0xcafe;
}
}

int main() {
    locally_defined_function();
    return 0;
}
