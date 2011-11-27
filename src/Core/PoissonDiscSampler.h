/**
 * Copyright (c) Bo Zhou<bo.schwarzstein@gmail.com>
 * An 2D implementation of "Fast Poisson Disk Sampling in Arbitrary Dimensions, R. Bridson, ACM SIGGRAPH 2007 Sketches Program".
 * Anybody could use this code freely, if you feel it good or helpful, please tell me, thank you very much.
 */
 
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef FPDS_HPP
#define FPDS_HPP
 
#include <math.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <math.h>
#include <boost/multi_array.hpp>
#include <Eigen/Eigen>
#include <RaytraceCommon.h>

namespace Raytrace {

// Define our basis data structure.

// Use C standard rand() to get uniform random number in (0,1)
inline float URand()
{
    return (float)rand() / (float)RAND_MAX;
}

static float FastPoissonDiskSampling(const float r, std::vector<Vector2>& PointList)
{
	typedef Vector2u Index2D;
	typedef Vector2 Point2D;

    PointList.clear();
    
    const int N = 2;
    const float SqrtN = sqrtf((float)N);
    const float R = r*SqrtN; // This R should be used to draw circle.
    const float RDivSqrtN = R / SqrtN;
    const int K = 30; // Paper suggested.
    const float CellSize = (float)N*RDivSqrtN;
    const int GridSize = (int)ceilf(1.0f/CellSize);
    const int INVALID_IDX = -1;
    
    // Allocate a grid to accelerate.
    // Each cell could have one and only one sampler.
    boost::multi_array<int,N> Grid( boost::extents[GridSize][GridSize] );
    memset( Grid.data(), INVALID_IDX, sizeof(int)*GridSize*GridSize );
    
    std::vector<int> ActiveList;
    
    Point2D P0;
    Index2D I0;
    
    P0[0] = URand();
    P0[1] = URand();
    I0[0] = (int)floorf( P0[0]*GridSize );
    I0[1] = (int)floorf( P0[1]*GridSize );
    
    ActiveList.push_back(0);
    PointList.push_back(P0);
    Grid[I0[0]][I0[1]] = 0;
    
    int Done = 1;
    while( ActiveList.size() != 0 )
    {
        // Initialize a sampler.
        size_t MagicIdx = (size_t)floorf( URand() * ActiveList.size() );
        size_t StartIdx = ActiveList[MagicIdx];
        Point2D StartPoint = PointList[ StartIdx ];

        bool Found = false;

        for( size_t i=0; i<K; ++i )
        {
            // Generate point in radius (R,2R)
            float t = URand()*M_PI*2.0;
            float r = URand()*R+R;
            float X = r*cosf( t );
            float Y = r*sinf( t );

            // Move to current center
            Point2D CurrentPoint;
            CurrentPoint[0] = X+StartPoint[0];  
            CurrentPoint[1] = Y+StartPoint[1];

            // Discard if out of domain
            if( CurrentPoint[0] < 0.0f || CurrentPoint[0] >= 1.0f )
                continue;

            if( CurrentPoint[1] < 0.0f || CurrentPoint[1] >= 1.0f )
                continue;

            // Which cell the test point located in
            Index2D TargetCell;
            TargetCell[0] = (int)floorf( CurrentPoint[0] / CellSize );
            TargetCell[1] = (int)floorf( CurrentPoint[1] / CellSize );

            if( TargetCell[0] < 0 || TargetCell[0] >= GridSize )
                continue;

            if( TargetCell[1] < 0 || TargetCell[1] >= GridSize )
                continue;


            if( Grid[TargetCell[0]][TargetCell[1]] != INVALID_IDX )
                continue;

            Index2D TCLeft( TargetCell );
            Index2D TCRight( TargetCell );
            Index2D TCDown( TargetCell );
            Index2D TCUp( TargetCell );

            Index2D TCLU( TargetCell );
            Index2D TCRU( TargetCell );
            Index2D TCLD( TargetCell );
            Index2D TCRD( TargetCell );

            bool A = false, B = false, C = false, D = false;
            bool E = false, F = false, G = false, H = false;

            TCLeft[0]--;
            if( TCLeft[0] > -1 )
            {
                int Idx2 = Grid[TCLeft[0]][TCLeft[1]];
                if( Idx2 >= 0 )
                {
					if( ( PointList[Idx2] - CurrentPoint ).norm() > R )
                    {
                        A = true;
                    }
                }else
                {
                    A = true;
                }
            }else
            {
                A = true;
            }

            TCRight[0]++;
            if( TCRight[0] < GridSize )
            {
                int Idx2 = Grid[TCRight[0]][TCRight[1]];
                if( Idx2 >= 0 )
                {
                    if( (PointList[Idx2] - CurrentPoint ).norm() > R )
                    {
                        B = true;
                    }
                }else
                {
                    B = true;
                }
            }else
            {
                B = true;
            }

            TCDown[1]--;
            if( TCDown[1] > -1 )
            {
                int Idx2 = Grid[TCDown[0]][TCDown[1]];
                if( Idx2 >= 0 )
                {
                    if( (PointList[Idx2] - CurrentPoint ).norm() > R )
                    {
                        C = true;
                    }
                }else
                {
                    C = true;
                }
            }else
            {
                C = true;
            }

            TCUp[1]++;
            if( TCUp[1] < GridSize )
            {
                int Idx2 = Grid[TCUp[0]][TCUp[1]];
                if( Idx2 >= 0 )
                {
                    if( ( PointList[Idx2] - CurrentPoint ).norm() > R )
                    {
                        D = true;
                    }
                }else
                {
                    D = true;
                }
            }else
            {
                D = true;
            }

            // 4 Corner
            // Left Up
            TCLU[0]--;TCLU[1]++;
            if( TCLU[0] > -1 && TCLU[1] < GridSize )
            {

                int Idx2 = Grid[TCLU[0]][TCLU[1]];
                
                if( Idx2 >= 0 )
                {
                    if( ( PointList[Idx2] - CurrentPoint ).norm() > R )
                    {
                        E = true;
                    }
                }else
                {
                    E = true;
                }
            }else
            {
                E = true;
            }

            // Right Up
            TCRU[0]++;TCRU[1]++;
            if( TCRU[0] < GridSize && TCRU[1] < GridSize )
            {

                int Idx2 = Grid[TCRU[0]][TCRU[1]];
                
                if( Idx2 >= 0 )
                {
                    if( ( PointList[Idx2] - CurrentPoint ).norm() > R )
                    {
                        F = true;
                    }
                }else
                {
                    F = true;
                }
            }else
            {
                F = true;
            }

            // Left Bottom
            TCLD[0]--;TCLD[1]--;
            if( TCLD[0] > -1 && TCLD[1] > -1 )
            {
                int Idx2 = Grid[TCLD[0]][TCLD[1]];
                if( Idx2 >= 0 )
                {
                    if( ( PointList[Idx2] - CurrentPoint ).norm() > R )
                    {
                        G = true;
                    }
                }else
                {
                    G = true;
                }
            }else
            {
                G = true;
            }

            // Right Bottom
            TCRD[0]++;TCRD[1]--;
            if( TCRD[0] < GridSize && TCRD[1] > -1 )
            {
                int Idx2 = Grid[TCRD[0]][TCRD[1]];
                if( Idx2 >= 0 )
                {
                    if( ( PointList[Idx2] - CurrentPoint ).norm() > R )
                    {
                        H = true;
                    }
                }else
                {
                    H = true;
                }
            }else
            {
                H = true;
            }

            if( A&B&C&D&E&F&G&H )
            {
                Grid[TargetCell[0]][TargetCell[1]] = Done;
                PointList.push_back(CurrentPoint);
                ActiveList.push_back(Done);            

                ++Done;
                Found = true;
                break;
            }
        }

        // We have to remove this test sampler from active list.
        if( Found == false && ActiveList.size() > 0 )
            ActiveList.erase( ActiveList.begin()+MagicIdx );
    }

    return R;
}


}

#endif