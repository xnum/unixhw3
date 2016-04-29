#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <set>

#include <signal.h>

#include "Executor.h"
#include "Parser.h"
#include "InputHandler.h"

vector<Executor> g_exes;

void childExit(int sig) {
    int status = 0;
    pid_t pid = wait(&status);
    if( pid < 0 ) {
        printf("wait error\n");
    }
    else {
        printf("catch a child %d\n",pid);
    }

    for( size_t i = 0 ; i < g_exes.size() ; ++i ) {
        if( g_exes[i].pid == pid )
            g_exes[i].done = true;
    }

    return;
}

int main()
{
    signal(SIGCHLD, childExit);
    //char* const envp[] = { 0, NULL };
    string line;

    InputHandler InHnd;
    while( 1 ) {
        cout << "$ ";
        line = InHnd.Getline();
        auto cmds = Parser::Parse(line);

        for( size_t i = 0 ; i < cmds.size() ; ++i ) {
            cout << cmds[i] << endl;
            Executor exec(cmds[i]);
            g_exes.push_back(exec);
        }

        for( size_t i = 0 ; i < g_exes.size()-1 ; ++i ) {
            g_exes[i].PipeWith(g_exes[i+1]);
        }

        for( size_t i = 0 ; i < g_exes.size() ; ++i ) {
            g_exes[i].Start();
        }

        set<int> closed_fd;
        for( size_t i = 0 ; i < g_exes.size() ; ++i ) {
            for( int j = 0 ; j < 2 ; ++j ) {
                for( int k = 0 ; k < 2 ; ++k ) {
                    const bool is_in = closed_fd.find(g_exes[i].fd[j][k]) != closed_fd.end();
                    if( g_exes[i].fd[j][k] != -1 && !is_in ) {
                        printf("free %\n",g_exes[i].fd[j][k]);
                        close(g_exes[i].fd[j][k]);
                        closed_fd.insert(g_exes[i].fd[j][k]);
                    }
                }
            }
        }

        while( 1 ) {
            sleep(1);
            bool ok = true;
            for( size_t i = 0 ; i < g_exes.size() ; ++i ) {
                if( g_exes[i].done == false )
                    ok = false;
            }
            if(ok)break;
        }

    }
    return 0;
}
