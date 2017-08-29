clang -emit-llvm -S a.c b.c
llvm-link -S a.ll b.ll -o all.ll
