# include <cstdlib>
# include <iostream>
# include <cmath>
# include <ctime>
# include <omp.h>
#include "TestOpenMp.h"
using namespace std;


//****************************************************************************80

int TestOpenMP::run ( )

//****************************************************************************80
//
//  Purpose:
//
//    MAIN is the main program for MXM_OPENMP.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    13 October 2011
//
//  Author:
//
//    John Burkardt
//
{
    double a[500][500];
    double angle;
    double b[500][500];
    double c[500][500];
    int i;
    int j;
    int k;
    int n = 500;
    double pi = 3.141592653589793;
    double s;
    int thread_num;
    double wtime;

    timestamp ( );

    cout << "\n";
    cout << "MXM_OPENMP:\n";
    cout << "  C++/OpenMP version\n";
    cout << "  Compute matrix product C = A * B.\n";

    thread_num = omp_get_max_threads ( );

    cout << "\n";
    cout << "  The number of processors available = " << omp_get_num_procs ( ) << "\n";
    cout << "  The number of threads available    = " << thread_num <<  "\n";

    cout << "  The matrix order N                 = " << n << "\n";
//
//  Loop 1: Evaluate A.
//
    s = 1.0 / sqrt ( ( double ) ( n ) );


# pragma omp parallel shared ( a, b, c, n, pi, s ) private ( angle, i, j, k )
    {
# pragma omp for
        for ( i = 0; i < n; i++ )
        {
            for ( j = 0; j < n; j++ )
            {
                angle = 2.0 * pi * i * j / ( double ) n;
                a[i][j] = s * ( sin ( angle ) + cos ( angle ) );

            }
        }
//
//  Loop 2: Copy A into B.
//

# pragma omp for
        for ( i = 0; i < n; i++ )
        {
            for ( j = 0; j < n; j++ )
            {
                b[i][j] = a[i][j];
                std::cout << i << "\t" << omp_get_thread_num() << "\t" << omp_get_num_threads() << "\t" << omp_in_parallel() << std::endl;
            }
        }
//
//  Loop 3: Compute C = A * B.
//
# pragma omp for
        for ( i = 0; i < n; i++ )
        {
            for ( j = 0; j < n; j++ )
            {
                c[i][j] = 0.0;
                for ( k = 0; k < n; k++ )
                {
                    c[i][j] = c[i][j] + a[i][k] * b[k][j];
                }
            }
        }

    }
    wtime = omp_get_wtime ( ) - wtime;
    cout << "  Elapsed seconds = " << wtime << "\n";
    cout << "  C(100,100)  = " << c[99][99] << "\n";
//
//  Terminate.
//
    cout << "\n";
    cout << "MXM_OPENMP:\n";
    cout << "  Normal end of execution.\n";

    cout << "\n";
    timestamp ( );

    return 0;
}
//****************************************************************************80

void TestOpenMP:: timestamp ( )

//****************************************************************************80
//
//  Purpose:
//
//    TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//
//    31 May 2001 09:45:54 AM
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    08 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    None
//
{
# define TIME_SIZE 40

    static char time_buffer[TIME_SIZE];
    const struct std::tm *tm_ptr;
    std::time_t now;

    now = std::time ( NULL );
    tm_ptr = std::localtime ( &now );

    std::strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm_ptr );

    std::cout << time_buffer << "\n";

    return;
# undef TIME_SIZE
}