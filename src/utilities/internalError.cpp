//
// Created by GentlyGuitar on 6/7/2017.
//

#include <exception>
#include <cstdio>
#include <execinfo.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#ifdef __linux__
#include <dlfcn.h>		// for dladdr
#include <cxxabi.h>		// for __cxa_demangle
/* get REG_EIP from ucontext.h */

#include <ucontext.h>
#endif

#include <sstream>
#include <peripheral/sysArgs.h>
#include <asmParser/sysDict.h>
#include "internalError.h"
#include "macros.h"
#include "flags.h"

#ifdef __linux__

/** Print a demangled stack backtrace of the caller function to FILE* out. */
void print_demangled_stacktrace(FILE *out = stdout, unsigned int max_frames = 63)
{
    fprintf(out, "[faulty file: %s]\n", SysDict::filename().c_str());
    fprintf(out, "[thread: %llu]\n", pthread_self());
    fprintf(out, "=================== [stack trace] ===================\n");

    // storage array for stack trace address data
    void* addrlist[max_frames+1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

    if (addrlen == 0) {
        fprintf(out, "  <empty, possibly corrupt>\n");
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addrlen; i++)
    {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        std::string symbol_str(symbollist[i]);
        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p)
        {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset
            && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char* ret = abi::__cxa_demangle(begin_name,
                                            funcname, &funcnamesize, &status);
            if (status == 0) {
                funcname = ret; // use possibly realloc()-ed string
                fprintf(out, "  %s: %s+%s\n",
                        symbollist[i], funcname, begin_offset);
            }
            else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                fprintf(out, "  %s: %s()+%s\n",
                        symbollist[i], begin_name, begin_offset);
            }
        }
        else
        {
            // couldn't parse the line? print the whole line.
            fprintf(out, "  %s\n", symbollist[i]);
        }

        int pos1 = symbol_str.find('[');
        int pos2 = symbol_str.find(']');
        string addr = symbol_str.substr(pos1+1, pos2-pos1-1);
        int pos3 = symbol_str.find('(');
        string exe = symbol_str.substr(0, pos3);

        if (pos3 != string::npos &&
            exe.find("libc.so") == exe.npos
            && exe.find("libstdc++.so") == exe.npos
            && exe.find("libpthread.so") == exe.npos) {
            char syscom[1024];
            sprintf(syscom,"echo -n '  => '; addr2line %s -e `which %s  | head -n 1 | cut -d\"'\" -f 2`; ", addr.c_str(), exe.c_str());
            //zpl("%s", syscom);
            //last parameter is the filename of the symbol
            system(syscom);
        }
    }

    fprintf(out, "=====================================================\n");

    free(funcname);
    free(symbollist);
}

#endif

void Errors::die() {
    uninstall_sig_handlers();
    if (ParallelModule) {
        pthread_exit(0);
    }
    else {
        exit(1);
    }
}

void Errors::semantic_error_handler() {
    print_demangled_stacktrace();
    die();
}

//void Errors::sigsegv_handler(int sig) {
//    printf("Received signal %d\n", sig);
//    die();
//}

//void Errors::install_sig_handlers() {
//    signal(SIGINT, Errors::sigsegv_handler);
//}

void Errors::sigsegv_handler(int sig, siginfo_t *info, void *secret) {
    void *trace[16];
    char **messages = (char **)NULL;
    int i, trace_size = 0;
    ucontext_t *uc = (ucontext_t *)secret;

    /* Do something useful with siginfo_t */
    if (sig == SIGSEGV)
        printf("\nGot signal %d, faulty address is %p, "
                       "from %p\n", sig, info->si_addr,
               (void*)uc->uc_mcontext.gregs[REG_RIP]);
    else
        printf("\nGot signal %d\n", sig);

    print_demangled_stacktrace();

    //print_backtrace_symbols();
#ifdef __linux__
    //Backtrace();
#endif
    die();
}

void Errors::init() {
    install_sig_handlers();
}

void Errors::uninstall_sig_handlers() {
    signal(SIGABRT, SIG_DFL);
}

void Errors::install_sig_handlers() {
    struct sigaction act;
    memset (&act, '\0', sizeof(act));

    /* Use the sa_sigaction field because the handles has two additional parameters */
    act.sa_sigaction = &Errors::sigsegv_handler;

    /* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
    //act.sa_flags = SA_SIGINFO;
    act.sa_flags = SA_RESTART | SA_SIGINFO;

    if (sigaction(SIGSEGV, &act, NULL) < 0) {
        fprintf(stderr, "install signal handler failed\n, exit...");
        die();
    }

    if (sigaction(SIGABRT, &act, NULL) < 0) {
        fprintf(stderr, "install signal handler failed\n, exit...");
        die();
    }
}

void Errors::print_backtrace_symbols() {
#ifdef __linux__
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    backtrace_symbols_fd(array, size, STDERR_FILENO);
#endif
}