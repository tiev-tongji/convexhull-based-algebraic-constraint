/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/

#include "MapDrawer.h"
#include "MapPoint.h"
#include "KeyFrame.h"
#include "QuadricSLAM/EllipsoidVerterx.h"
#include "QuadricSLAM/Parameters.h"
#include "Converter.h"
#include <opencv2/core/eigen.hpp>
#include <Eigen/Core>
#include <pangolin/pangolin.h>
#include <mutex>

namespace ORB_SLAM2
{


MapDrawer::MapDrawer(Map* pMap, const string &strSettingPath):mpMap(pMap)
{
    cv::FileStorage fSettings(strSettingPath, cv::FileStorage::READ);

    mKeyFrameSize = fSettings["Viewer.KeyFrameSize"];
    mKeyFrameLineWidth = fSettings["Viewer.KeyFrameLineWidth"];
    mGraphLineWidth = fSettings["Viewer.GraphLineWidth"];
    mPointSize = fSettings["Viewer.PointSize"];
    mCameraSize = fSettings["Viewer.CameraSize"];
    mCameraLineWidth = fSettings["Viewer.CameraLineWidth"];

}

void MapDrawer::drawAxisNormal()
{
    float length = 2.0;
    // x
    glColor3f(1.0,0.0,0.0); // red x
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0f, 0.0f);
    glVertex3f(length, 0.0f, 0.0f);
    glEnd();

    // y 
    glColor3f(0.0,1.0,0.0); // green y
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0f, 0.0f);
    glVertex3f(0.0, length, 0.0f);

    glEnd();

    // z 
    glColor3f(0.0,0.0,1.0); // blue z
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0f ,0.0f );
    glVertex3f(0.0, 0.0f ,length );

    glEnd();
}

void MapDrawer::DrawMapPoints()
{
    const vector<MapPoint*> &vpMPs = mpMap->GetAllMapPoints();
    const vector<MapPoint*> &vpRefMPs = mpMap->GetReferenceMapPoints();

    set<MapPoint*> spRefMPs(vpRefMPs.begin(), vpRefMPs.end());

    if(vpMPs.empty())
        return;

    // draw all points
    glPointSize(mPointSize*2);
    glBegin(GL_POINTS);
    glColor3f(0.6,0.6,0.6);

    for(size_t i=0, iend=vpMPs.size(); i<iend;i++)
    {
        //if(vpMPs[i]->isBad() || spRefMPs.count(vpMPs[i]))
        if(vpMPs[i]->isBad())
            continue;
        cv::Mat pos = vpMPs[i]->GetWorldPos();
        glVertex3f(pos.at<float>(0),pos.at<float>(1),pos.at<float>(2));
    }
    glEnd();

    //draw reference points (red)
    // glPointSize(mPointSize*2);
    // glBegin(GL_POINTS);
    // glColor3f(1.0,0.0,0.0);
    // for(set<MapPoint*>::iterator sit=spRefMPs.begin(), send=spRefMPs.end(); sit!=send; sit++)
    // {
    //     if((*sit)->isBad())
    //         continue;
    //     cv::Mat pos = (*sit)->GetWorldPos();
    //     glVertex3f(pos.at<float>(0),pos.at<float>(1),pos.at<float>(2));

    // }

    // glEnd();

    // draw map points of map objects
    if(paras.draw_object_points)
    {
        glPointSize(mPointSize * 3);
        glBegin(GL_POINTS);
        const vector<MapObject*> vpMOs = mpMap->GetAllMapObjects();
        for(auto pMO: vpMOs)
        {
            if(pMO->mpQuadric->isColorSet())
            {
                Eigen::Vector3d color3d = pMO->mpQuadric->getColor();
                color3d = color3d/255.0;
                glColor3f(color3d(2),color3d(1),color3d(0));
            }
            else
            {
                glColor3f(0.0,0.0,0.0);
            }
            auto asso_points = pMO->GetAssociatedMP();
            for(auto pMP : asso_points)
            {
                if(pMP->isBad()) continue;
                cv::Mat pos = pMP->GetWorldPos();
                if(pos.rows == 0) continue;
                glVertex3f(pos.at<float>(0), pos.at<float>(1), pos.at<float>(2));
            }
        }
        glEnd();
    }
}

void MapDrawer::drawCuboids()
{
    std::vector<g2o::ellipsoid*> ellipsoids;
    std::vector<MapObject*> vpMOs= mpMap->GetAllMapObjects();
    for(auto pMO : vpMOs)
    {
        if(pMO ->isBad() || !pMO->mbIsInitialized) continue;
        ellipsoids.push_back(pMO->mpQuadric);
    }

    //std::vector<g2o::ellipsoid*> ellipsoidsVisual = mpMap->GetAllEllipsoidsVisual();
    //ellipsoids.insert(ellipsoids.end(), ellipsoidsVisual.begin(), ellipsoidsVisual.end());

    for( size_t i=0; i<ellipsoids.size(); i++)
    {
        Eigen::Matrix3Xd corners = ellipsoids[i]->compute3D_BoxCorner();

        glPushMatrix();

        glLineWidth(mCameraLineWidth);

        if(ellipsoids[i]->isColorSet()){
            Eigen::Vector3d color = ellipsoids[i]->getColor();
            glColor3f(1.0f,0.0f,0.0f);  // red color.
        }
        else
            glColor3f(0.0f,0.0f,1.0f);
        
        glBegin(GL_LINES);

        // draw cube lines. 
        for(int m=0;m<corners.cols();m++){
            for( int n=m+1; n<corners.cols();n++)
            {
                int m_first = m;
                glVertex3f(corners(0,m_first),corners(1,m_first),corners(2,m_first));
                int m_next=n;
                glVertex3f(corners(0,m_next),corners(1,m_next),corners(2,m_next));
            }
        }
        glEnd();
        glPopMatrix();
    }
}

void MapDrawer::drawEllipsoids() {
    std::vector<g2o::ellipsoid*> ellipsoids;
    std::vector<MapObject*> vpMOs= mpMap->GetAllMapObjects();
    for(auto pMO : vpMOs)
    {
        // if(pMO->isBad() || pMO->mbIsInitialized) continue;
        // set color to ellipsoid
        cv::Scalar s_color = paras.color_vec[pMO->mnClassId%7];
        Eigen::Vector3d color;
        color << s_color[0], s_color[1], s_color[2];
        pMO->mpQuadric->setColor(color, 1.0);
        if(pMO->mbIsInitialized){
            ellipsoids.push_back(pMO->mpQuadric);
        }
            
    }

    //std::vector<g2o::ellipsoid*> ellipsoidsVisual = mpMap->GetAllEllipsoidsVisual();
    //ellipsoids.insert(ellipsoids.end(), ellipsoidsVisual.begin(), ellipsoidsVisual.end());
    for( size_t i=0; i<ellipsoids.size(); i++)
    {
        g2o::SE3Quat se3_Twc = ellipsoids[i]->pose;
        cv::Mat Twc = Converter::toCvMat(se3_Twc).t();
        Eigen::Vector3d scale = ellipsoids[i]->scale;

        glLineWidth(2.0);
        Eigen::Vector3d color3d = ellipsoids[i]->getColor();
        color3d = color3d/255.0;
        glColor3f(color3d(2), color3d(1), color3d(0));

        GLUquadricObj *pObj;
        pObj = gluNewQuadric();
        gluQuadricDrawStyle(pObj, GLU_LINE);
        
        glPushMatrix();
        glMultMatrixf(Twc.ptr<GLfloat>(0));
        glScaled(scale[0],scale[1],scale[2]);
        
        glBegin(GL_COMPILE);
        gluQuadricDrawStyle(pObj, GLU_LINE);
        gluQuadricNormals(pObj, GLU_NONE);
        // draw a sphere with radius 1.0, center (0,0,0), slices 15, and stacks 10.
        gluSphere(pObj, 1.0, 14, 9);
        //drawAxisNormal();
        glEnd();
        glPopMatrix();
    }
}

void MapDrawer::DrawKeyFrames(const bool bDrawKF, const bool bDrawGraph)
{
    const float &w = mKeyFrameSize;
    const float h = w*0.75;
    const float z = w*0.6;

    const vector<KeyFrame*> vpKFs = mpMap->GetAllKeyFrames();

    if(bDrawKF)
    {
        for(size_t i=0; i<vpKFs.size(); i++)
        {
            KeyFrame* pKF = vpKFs[i];
            cv::Mat Twc = pKF->GetPoseInverse().t();

            glPushMatrix();

            glMultMatrixf(Twc.ptr<GLfloat>(0));

            glLineWidth(mKeyFrameLineWidth);
            glColor3f(0.0f,0.0f,1.0f);
            glBegin(GL_LINES);
            glVertex3f(0,0,0);
            glVertex3f(w,h,z);
            glVertex3f(0,0,0);
            glVertex3f(w,-h,z);
            glVertex3f(0,0,0);
            glVertex3f(-w,-h,z);
            glVertex3f(0,0,0);
            glVertex3f(-w,h,z);

            glVertex3f(w,h,z);
            glVertex3f(w,-h,z);

            glVertex3f(-w,h,z);
            glVertex3f(-w,-h,z);

            glVertex3f(-w,h,z);
            glVertex3f(w,h,z);

            glVertex3f(-w,-h,z);
            glVertex3f(w,-h,z);
            glEnd();

            glPopMatrix();
        }
    }

    if(bDrawGraph)
    {
        glLineWidth(mGraphLineWidth);
        glColor4f(0.0f,1.0f,0.0f,0.6f);
        glBegin(GL_LINES);

        for(size_t i=0; i<vpKFs.size(); i++)
        {
            // Covisibility Graph
            const vector<KeyFrame*> vCovKFs = vpKFs[i]->GetCovisiblesByWeight(100);
            cv::Mat Ow = vpKFs[i]->GetCameraCenter();
            if(!vCovKFs.empty())
            {
                for(vector<KeyFrame*>::const_iterator vit=vCovKFs.begin(), vend=vCovKFs.end(); vit!=vend; vit++)
                {
                    if((*vit)->mnId<vpKFs[i]->mnId)
                        continue;
                    cv::Mat Ow2 = (*vit)->GetCameraCenter();
                    glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                    glVertex3f(Ow2.at<float>(0),Ow2.at<float>(1),Ow2.at<float>(2));
                }
            }

            // Spanning tree
            KeyFrame* pParent = vpKFs[i]->GetParent();
            if(pParent)
            {
                cv::Mat Owp = pParent->GetCameraCenter();
                glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                glVertex3f(Owp.at<float>(0),Owp.at<float>(1),Owp.at<float>(2));
            }

            // Loops
            set<KeyFrame*> sLoopKFs = vpKFs[i]->GetLoopEdges();
            for(set<KeyFrame*>::iterator sit=sLoopKFs.begin(), send=sLoopKFs.end(); sit!=send; sit++)
            {
                if((*sit)->mnId<vpKFs[i]->mnId)
                    continue;
                cv::Mat Owl = (*sit)->GetCameraCenter();
                glVertex3f(Ow.at<float>(0),Ow.at<float>(1),Ow.at<float>(2));
                glVertex3f(Owl.at<float>(0),Owl.at<float>(1),Owl.at<float>(2));
            }
        }

        glEnd();
    }
}

void MapDrawer::DrawCurrentCamera(pangolin::OpenGlMatrix &Twc)
{
    const float &w = mCameraSize;
    const float h = w*0.75;
    const float z = w*0.6;

    glPushMatrix();

#ifdef HAVE_GLES
        glMultMatrixf(Twc.m);
#else
        glMultMatrixd(Twc.m);
#endif

    glLineWidth(mCameraLineWidth);
    glColor3f(0.0f,1.0f,0.0f);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(w,h,z);
    glVertex3f(0,0,0);
    glVertex3f(w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,-h,z);
    glVertex3f(0,0,0);
    glVertex3f(-w,h,z);

    glVertex3f(w,h,z);
    glVertex3f(w,-h,z);

    glVertex3f(-w,h,z);
    glVertex3f(-w,-h,z);

    glVertex3f(-w,h,z);
    glVertex3f(w,h,z);

    glVertex3f(-w,-h,z);
    glVertex3f(w,-h,z);
    glEnd();

    glPopMatrix();
}

void MapDrawer::SetCurrentCameraPose(const cv::Mat &Tcw)
{
    unique_lock<mutex> lock(mMutexCamera);
    mCameraPose = Tcw.clone();
}

void MapDrawer::GetCurrentOpenGLCameraMatrix(pangolin::OpenGlMatrix &M)
{
    if(!mCameraPose.empty())
    {
        cv::Mat Rwc(3,3,CV_32F);
        cv::Mat twc(3,1,CV_32F);
        {
            unique_lock<mutex> lock(mMutexCamera);
            Rwc = mCameraPose.rowRange(0,3).colRange(0,3).t();
            twc = -Rwc*mCameraPose.rowRange(0,3).col(3);
        }

        M.m[0] = Rwc.at<float>(0,0);
        M.m[1] = Rwc.at<float>(1,0);
        M.m[2] = Rwc.at<float>(2,0);
        M.m[3]  = 0.0;

        M.m[4] = Rwc.at<float>(0,1);
        M.m[5] = Rwc.at<float>(1,1);
        M.m[6] = Rwc.at<float>(2,1);
        M.m[7]  = 0.0;

        M.m[8] = Rwc.at<float>(0,2);
        M.m[9] = Rwc.at<float>(1,2);
        M.m[10] = Rwc.at<float>(2,2);
        M.m[11]  = 0.0;

        M.m[12] = twc.at<float>(0);
        M.m[13] = twc.at<float>(1);
        M.m[14] = twc.at<float>(2);
        M.m[15]  = 1.0;
    }
    else
        M.SetIdentity();
}

// void MapDrawer::SE3ToOpenGLCameraMatrix(g2o::SE3Quat &matInSe3, pangolin::OpenGlMatrix &M)
// {
//     // eigen to cv
//     Eigen::Matrix4d matEigen = matInSe3.to_homogeneous_matrix();
//     cv::Mat matIn;
//     eigen2cv(matEigen, matIn);

//     if(!matIn.empty())
//     {
//         cv::Mat Rwc(3,3,CV_64F);
//         cv::Mat twc(3,1,CV_64F);
//         {
//             unique_lock<mutex> lock(mMutexCamera);
//             Rwc = matIn.rowRange(0,3).colRange(0,3).t();
//             twc = -Rwc*matIn.rowRange(0,3).col(3);
//         }

//         M.m[0] = Rwc.at<double>(0,0);
//         M.m[1] = Rwc.at<double>(1,0);
//         M.m[2] = Rwc.at<double>(2,0);
//         M.m[3]  = 0.0;

//         M.m[4] = Rwc.at<double>(0,1);
//         M.m[5] = Rwc.at<double>(1,1);
//         M.m[6] = Rwc.at<double>(2,1);
//         M.m[7]  = 0.0;

//         M.m[8] = Rwc.at<double>(0,2);
//         M.m[9] = Rwc.at<double>(1,2);
//         M.m[10] = Rwc.at<double>(2,2);
//         M.m[11]  = 0.0;

//         M.m[12] = twc.at<double>(0);
//         M.m[13] = twc.at<double>(1);
//         M.m[14] = twc.at<double>(2);
//         M.m[15]  = 1.0;
//     }
//     else
//         M.SetIdentity();
// }

// // not inverse, keep origin
// void MapDrawer::SE3ToOpenGLCameraMatrixOrigin(g2o::SE3Quat &matInSe3, pangolin::OpenGlMatrix &M)
// {
//     // eigen to cv
//     Eigen::Matrix4d matEigen = matInSe3.to_homogeneous_matrix();
//     cv::Mat matIn;
//     eigen2cv(matEigen, matIn);

//     if(!matIn.empty())
//     {
//         cv::Mat Rwc(3,3,CV_64F);
//         cv::Mat twc(3,1,CV_64F);
//         {
//             unique_lock<mutex> lock(mMutexCamera);
//             Rwc = matIn.rowRange(0,3).colRange(0,3);
//             twc = matIn.rowRange(0,3).col(3);
//         }

//         M.m[0] = Rwc.at<double>(0,0);
//         M.m[1] = Rwc.at<double>(1,0);
//         M.m[2] = Rwc.at<double>(2,0);
//         M.m[3]  = 0.0;

//         M.m[4] = Rwc.at<double>(0,1);
//         M.m[5] = Rwc.at<double>(1,1);
//         M.m[6] = Rwc.at<double>(2,1);
//         M.m[7]  = 0.0;

//         M.m[8] = Rwc.at<double>(0,2);
//         M.m[9] = Rwc.at<double>(1,2);
//         M.m[10] = Rwc.at<double>(2,2);
//         M.m[11]  = 0.0;

//         M.m[12] = twc.at<double>(0);
//         M.m[13] = twc.at<double>(1);
//         M.m[14] = twc.at<double>(2);
//         M.m[15]  = 1.0;
//     }
//     else
//         M.SetIdentity();
// }

} //namespace ORB_SLAM
