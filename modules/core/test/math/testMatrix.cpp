/****************************************************************************
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2017 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://visp.inria.fr for more information.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Test some vpMatrix functionalities.
 *
 * Authors:
 * Fabien Spindler
 *
 *****************************************************************************/

/*!
  \example testMatrix.cpp

  Test some vpMatrix functionalities.
*/

#include <visp3/core/vpConfig.h>
#include <visp3/core/vpDebug.h>
#include <visp3/core/vpMath.h>
#include <visp3/core/vpHomogeneousMatrix.h>
#include <visp3/core/vpVelocityTwistMatrix.h>
#include <visp3/core/vpGEMM.h>

#include <stdlib.h>
#include <stdio.h>


namespace {
  bool test(const std::string &s, const vpMatrix &M, const std::vector<double> &bench)
  {
    static unsigned int cpt = 0;
    std::cout << "** Test " << ++cpt << std::endl;
    std::cout << s << "(" << M.getRows() << "," << M.getCols() << ") = \n" << M << std::endl;
    if(bench.size() != M.size()) {
      std::cout << "Test fails: bad size wrt bench" << std::endl;
      return false;
    }
    for (unsigned int i=0; i<M.size(); i++) {
      if (std::fabs(M.data[i]-bench[i]) > std::fabs(M.data[i])*std::numeric_limits<double>::epsilon()) {
        std::cout << "Test fails: bad content" << std::endl;
        return false;
      }
    }

    return true;
  }

  double getRandomValues(const double min, const double max) {
    return (max - min) * ( (double) rand() / (double) RAND_MAX ) + min;
  }

  bool equalMatrix(const vpMatrix &A, const vpMatrix &B) {
    if (A.getRows() != B.getRows() || A.getCols() != B.getCols()) {
      return false;
    }

    for (unsigned int i = 0; i < A.getRows(); i++) {
      for (unsigned int j = 0; j < A.getCols(); j++) {
        if ( !vpMath::equal(A[i][j], B[i][j], std::numeric_limits<double>::epsilon()) ) {
          return false;
        }
      }
    }

    return true;
  }
}


int
main()
{
  try {
    int err = 1;
    {
      vpColVector c(6, 1);
      vpRowVector r(6, 1);
      std::vector<double> bench(6, 1);
      vpMatrix M1(c);
      if (test("M1", M1, bench) == false)
        return err;
      vpMatrix M2(r);
      if (test("M2", M2, bench) == false)
        return err;
    }
    {
      vpMatrix M(4,5);
      int val = 0;
      for(unsigned int i=0; i<M.getRows(); i++) {
        for(unsigned int j=0; j<M.getCols(); j++) {
          M[i][j] = val++;
        }
      }
      std::cout <<"M ";
      M.print (std::cout, 4);

      vpMatrix N;
      N.init(M, 0, 1, 2, 3);
      std::cout <<"N ";
      N.print (std::cout, 4);
      std::string header("My 4-by-5 matrix\nwith a second line");

      // Save matrix in text format
      if (vpMatrix::saveMatrix("matrix.mat", M, false, header.c_str()))
        std::cout << "Matrix saved in matrix.mat file" << std::endl;
      else
        return err;

      // Load matrix in text format
      vpMatrix M1;
      char header_[100];
      if (vpMatrix::loadMatrix("matrix.mat", M1, false, header_))
        std::cout << "Matrix loaded from matrix.mat file with header \"" << header_ << "\": \n" << M1 << std::endl;
      else
        return err;
      if (header != std::string(header_)) {
        std::cout << "Bad header in matrix.mat" << std::endl;
        return err;
      }

      // Save matrix in binary format
      if (vpMatrix::saveMatrix("matrix.bin", M, true, header.c_str()))
        std::cout << "Matrix saved in matrix.bin file" << std::endl;
      else
        return err;

      // Load matrix in binary format
      if (vpMatrix::loadMatrix("matrix.bin", M1, true, header_))
        std::cout << "Matrix loaded from matrix.bin file with header \"" << header_ << "\": \n" << M1 << std::endl;
      else
        return err;
      if (header != std::string(header_)) {
        std::cout << "Bad header in matrix.bin" << std::endl;
        return err;
      }

      // Save matrix in YAML format
      if (vpMatrix::saveMatrixYAML("matrix.yml", M, header.c_str()))
        std::cout << "Matrix saved in matrix.yml file" << std::endl;
      else
        return err;

      // Read matrix in YAML format
      vpMatrix M2;
      if (vpMatrix::loadMatrixYAML("matrix.yml", M2, header_))
        std::cout << "Matrix loaded from matrix.yml file with header \"" << header_ << "\": \n" << M2 << std::endl;
      else
        return err;
      if (header != std::string(header_)) {
        std::cout << "Bad header in matrix.mat" << std::endl;
        return err;
      }
    }

    {
      vpRotationMatrix R(vpMath::rad(10), vpMath::rad(20), vpMath::rad(30));
      std::cout << "R: \n" << R << std::endl;
      vpMatrix M1(R);
      std::cout << "M1: \n" << M1 << std::endl;
      vpMatrix M2(M1);
      std::cout << "M2: \n" << M2 << std::endl;
      vpMatrix M3 = R;
      std::cout << "M3: \n" << M3 << std::endl;
      vpMatrix M4 = M1;
      std::cout << "M4: \n" << M4 << std::endl;
    }
    {

      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST PRETTY PRINT---" << std::endl;
      std::cout << "------------------------" << std::endl;
      vpMatrix M ;
      M.eye(4);

      std::cout << "call std::cout << M;" << std::endl;
      std::cout << M << std::endl;

      std::cout << "call M.print (std::cout, 4);" << std::endl;
      M.print (std::cout, 4);

      std::cout << "------------------------" << std::endl;
      M.resize(3,3) ;
      M.eye(3);
      M[1][0]=1.235;
      M[1][1]=12.345;
      M[1][2]=.12345;
      std::cout << "call std::cout << M;" << std::endl;
      std::cout << M;
      std::cout << "call M.print (std::cout, 6);" << std::endl;
      M.print (std::cout, 6);
      std::cout << std::endl;

      std::cout << "------------------------" << std::endl;
      M[0][0]=-1.235;
      M[1][0]=-12.235;

      std::cout << "call std::cout << M;" << std::endl;
      std::cout << M << std::endl;

      std::cout << "call M.print (std::cout, 10);" << std::endl;
      M.print (std::cout, 10);
      std::cout << std::endl;

      std::cout << "call M.print (std::cout, 2);" << std::endl;
      M.print (std::cout, 2);
      std::cout << std::endl;

      std::cout << "------------------------" << std::endl;
      M.resize(3,3) ;
      M.eye(3);
      M[0][2]=-0.0000000876;
      std::cout << "call std::cout << M;" << std::endl;
      std::cout << M << std::endl;

      std::cout << "call M.print (std::cout, 4);" << std::endl;
      M.print (std::cout, 4);
      std::cout << std::endl;
      std::cout << "call M.print (std::cout, 10, \"M\");" << std::endl;
      M.print (std::cout, 10, "M");
      std::cout << std::endl;
      std::cout << "call M.print (std::cout, 20, \"M\");" << std::endl;
      M.print (std::cout, 20, "M");
      std::cout << std::endl;


      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST RESIZE --------" << std::endl;
      std::cout << "------------------------" << std::endl;
      std::cout <<  "5x5" << std::endl;
      M.resize(5,5,false);
      std::cout << M << std::endl;
      std::cout << "3x2" << std::endl;
      M.resize(3,2,false);
      std::cout << M << std::endl;
      std::cout << "2x2" << std::endl;
      M.resize(2,2,false);
      std::cout << M << std::endl;
      std::cout << "------------------------" << std::endl;

      vpVelocityTwistMatrix vMe;
      vpMatrix A(1,6),B;

      A=1.0;
      //vMe=1.0;
      B=A*vMe;

      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpRowVector * vpColVector" << std::endl;
      std::cout << "------------------------" << std::endl;
      vpRowVector r(3);
      r[0] = 2;
      r[1] = 3;
      r[2] = 4;

      vpColVector c(3);
      c[0] = 1;
      c[1] = 2;
      c[2] = -1;

      double rc = r * c;

      r.print(std::cout, 2, "r");
      c.print(std::cout, 2, "c");
      std::cout << "r * c = " << rc << std::endl;

      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpRowVector * vpMatrix" << std::endl;
      std::cout << "------------------------" << std::endl;
      M.resize(3,3) ;
      M.eye(3);

      M[1][0] = 1.5;
      M[2][0] = 2.3;

      vpRowVector rM = r * M;

      r.print(std::cout, 2, "r");
      M.print(std::cout, 10, "M");
      std::cout << "r * M = " << rM << std::endl;

      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpGEMM " << std::endl;
      std::cout << "------------------------" << std::endl;
      M.resize(3,3) ;
      M.eye(3);
      vpMatrix N(3, 3);
      N[0][0] = 2;
      N[1][0] = 1.2;
      N[1][2] = 0.6;
      N[2][2] = 0.25;

      vpMatrix C(3, 3);
      C.eye(3);

      vpMatrix D;

      //realise the operation D = 2 * M^T * N + 3 C
      vpGEMM(M, N, 2, C, 3, D, VP_GEMM_A_T);
      std::cout << D << std::endl;
    }

    {
      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpMatrix insert() with same colNum " << std::endl;
      std::cout << "------------------------" << std::endl;
      const unsigned int nb = 100; //10000; //for ctest otherwise takes too long time with static call
      const unsigned int size = 100;

      vpMatrix m_big(nb*size, 6);
      std::vector<vpMatrix> submatrices(nb);
      for (size_t cpt = 0; cpt < submatrices.size(); cpt++) {
        vpMatrix m(size, 6);

        for (unsigned int i = 0; i < m.getRows(); i++) {
          for (unsigned int j = 0; j < m.getCols(); j++) {
            m[i][j] = getRandomValues(-100.0, 100.0);
          }
        }

        submatrices[cpt] = m;
      }

      double t = vpTime::measureTimeMs();
      for (unsigned int i = 0; i < nb; i++) {
        m_big.insert(submatrices[(size_t) i], i*size, 0);
      }
      t = vpTime::measureTimeMs() - t;
      std::cout << "Matrix insert(): " << t << " ms" << std::endl;

      for (unsigned int cpt = 0; cpt < nb; cpt++) {
        for (unsigned int i = 0; i <size; i++) {
          for (unsigned int j = 0; j < 6; j++) {
            if ( !vpMath::equal(m_big[cpt*size+i][j], submatrices[(size_t) cpt][i][j], std::numeric_limits<double>::epsilon()) ) {
              std::cerr << "Problem with vpMatrix insert()!" << std::endl;
              return EXIT_FAILURE;
            }
          }
        }
      }

      //Try to insert empty matrices
      vpMatrix m1(2,3), m2, m3;
      m1.insert(m2, 0, 0);
      m3.insert(m2, 0, 0);

      std::cout << "Insert empty matrices:" << std::endl;
      std::cout << "m1:\n" << m1 << std::endl;
      std::cout << "m2:\n" << m2 << std::endl;
      std::cout << "m3:\n" << m3 << std::endl;


      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpMatrix stack()" << std::endl;
      std::cout << "------------------------" << std::endl;

      {
        vpMatrix L, L2(2,6);
        L2 = 2;
        L.stack(L2);
        std::cout << "L:\n" << L << std::endl;
        L2.resize(3,6);
        L2 = 3;
        L.stack(L2);
        std::cout << "L:\n" << L << std::endl;
      }


      vpMatrix m_big_stack;
      t = vpTime::measureTimeMs();
      for (unsigned int i = 0; i < nb; i++) {
        m_big_stack.stack(submatrices[(size_t) i]);
      }
      t = vpTime::measureTimeMs() - t;
      std::cout << "\nMatrix stack(): " << t << " ms" << std::endl;

      if (!equalMatrix(m_big, m_big_stack)) {
        std::cerr << "Problem with vpMatrix stack()!" << std::endl;
        return EXIT_FAILURE;
      }


      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpMatrix stack(vpRowVector)" << std::endl;
      std::cout << "------------------------" << std::endl;

      vpMatrix m_big_stack_row;
      t = vpTime::measureTimeMs();
      for (unsigned int i = 0; i < m_big_stack.getRows(); i++) {
        m_big_stack_row.stack(m_big_stack.getRow(i));
      }
      t = vpTime::measureTimeMs() - t;
      std::cout << "\nMatrix stack(vpRowVector): " << t << " ms" << std::endl;

      if (!equalMatrix(m_big_stack, m_big_stack_row)) {
        std::cerr << "Problem with vpMatrix stack(vpRowVector)!" << std::endl;
        return EXIT_FAILURE;
      }


      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpMatrix::stack()" << std::endl;
      std::cout << "------------------------" << std::endl;

      {
        vpMatrix L, L2(2,6), L_tmp;
        L2 = 2;
        vpMatrix::stack(L_tmp, L2, L);
        std::cout << "L:\n" << L << std::endl;
        L2.resize(3,6);
        L2 = 3;
        L_tmp = L;
        vpMatrix::stack(L_tmp, L2, L);
        std::cout << "L:\n" << L << std::endl;
      }


      vpMatrix m_big_stack_static, m_big_stack_static_tmp;
      t = vpTime::measureTimeMs();
      for (unsigned int i = 0; i < nb; i++) {
        vpMatrix::stack(m_big_stack_static_tmp, submatrices[(size_t) i], m_big_stack_static);
        m_big_stack_static_tmp = m_big_stack_static;
      }
      t = vpTime::measureTimeMs() - t;
      std::cout << "\nMatrix::stack(): " << t << " ms" << std::endl;

      if (!equalMatrix(m_big, m_big_stack_static)) {
        std::cerr << "Problem with vpMatrix::stack()!" << std::endl;
        return EXIT_FAILURE;
      }


      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpMatrix::stack(vpMatrix, vpRowVector, vpMatrix)" << std::endl;
      std::cout << "------------------------" << std::endl;

      vpMatrix m_big_stack_static_row, m_big_stack_static_row_tmp;
      t = vpTime::measureTimeMs();
      for (unsigned int i = 0; i < m_big_stack_static.getRows(); i++) {
        vpMatrix::stack(m_big_stack_static_row_tmp, m_big_stack_static.getRow(i), m_big_stack_static_row);
        m_big_stack_static_row_tmp = m_big_stack_static_row;
      }
      t = vpTime::measureTimeMs() - t;
      std::cout << "\nMatrix::stack(vpMatrix, vpRowVector, vpMatrix): " << t << " ms" << std::endl;

      if (!equalMatrix(m_big_stack_static, m_big_stack_static_row)) {
        std::cerr << "Problem with vpMatrix::stack(vpMatrix, vpRowVector, vpMatrix)!" << std::endl;
        return EXIT_FAILURE;
      }
    }

    {
      vpMatrix m1(11,9), m2(3,4);
      for (unsigned int i = 0; i < m2.getRows(); i++) {
        for (unsigned int j = 0; j < m2.getCols(); j++) {
          m2[i][j] = getRandomValues(-100.0, 100.0);
        }
      }

      unsigned int offset_i = 4, offset_j = 3;
      m1.insert(m2, offset_i, offset_j);

      for (unsigned int i = 0; i < m2.getRows(); i++) {
        for (unsigned int j = 0; j < m2.getCols(); j++) {
          if ( !vpMath::equal(m1[i+offset_i][j+offset_j], m2[i][j], std::numeric_limits<double>::epsilon()) ) {
            std::cerr << "Problem with vpMatrix insert()!" << std::endl;
            return EXIT_FAILURE;
          }
        }
      }

      offset_i = 4, offset_j = 5;
      m1.insert(m2, offset_i, offset_j);

      for (unsigned int i = 0; i < m2.getRows(); i++) {
        for (unsigned int j = 0; j < m2.getCols(); j++) {
          if ( !vpMath::equal(m1[i+offset_i][j+offset_j], m2[i][j], std::numeric_limits<double>::epsilon()) ) {
            std::cerr << "Problem with vpMatrix insert()!" << std::endl;
            return EXIT_FAILURE;
          }
        }
      }

      offset_i = 8, offset_j = 5;
      m1.insert(m2, offset_i, offset_j);

      for (unsigned int i = 0; i < m2.getRows(); i++) {
        for (unsigned int j = 0; j < m2.getCols(); j++) {
          if ( !vpMath::equal(m1[i+offset_i][j+offset_j], m2[i][j], std::numeric_limits<double>::epsilon()) ) {
            std::cerr << "Problem with vpMatrix insert()!" << std::endl;
            return EXIT_FAILURE;
          }
        }
      }
    }

    {
      std::cout << "------------------------" << std::endl;
      std::cout << "--- TEST vpMatrix::juxtaposeMatrices()" << std::endl;
      std::cout << "------------------------" << std::endl;

      vpMatrix A(5, 6), B(5, 4);
      for (unsigned int i = 0; i < A.getRows(); i++) {
        for (unsigned int j = 0; j < A.getCols(); j++) {
          A[i][j] = i*A.getCols()+j;

          if (j < B.getCols()) {
            B[i][j] = (i*B.getCols()+j)*10;
          }
        }
      }

      vpMatrix juxtaposeM;
      vpMatrix::juxtaposeMatrices(A, B, juxtaposeM);
      std::cout << "juxtaposeM:\n" << juxtaposeM << std::endl;
    }

    std::cout << "All tests succeed" << std::endl;
    return EXIT_SUCCESS;
  }
  catch(vpException &e) {
    std::cout << "Catch an exception: " << e << std::endl;
    return EXIT_FAILURE;
  }
}

