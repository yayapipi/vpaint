// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "View3D.h"
#include "Scene.h"
#include "Timeline.h"
#include <QtDebug>
#include "Global.h"
#include "View.h"
#include "Background/Background.h"
#include "Background/BackgroundRenderer.h"

#include "VectorAnimationComplex/VAC.h"
#include "VectorAnimationComplex/KeyCell.h"

// define mouse actions

#define DRAW_ACTION 10
#define SELECT_ACTION 20
#define ADDSELECT_ACTION 21
#define DESELECT_ACTION 22
#define TOGGLESELECT_ACTION 23
#define DESELECTALL_ACTION 24

View3D::View3D(Scene *scene, QWidget *parent) :
    GLWidget(parent, false), // Difference from View here
    scene_(scene),
    displayedTimes_(),
    pickingImg_(0),
    //frame_(0),
    vac_(0)
{
    // Make renderers
    // XXX Make it work with layers
    //Background * bg = scene_->background();
    //backgroundRenderers_[bg] = new BackgroundRenderer(bg, context(), this);


    cameraTravellingIsEnabled_ = true;
    drawingIsEnable_ = false;

    // behave as a separate window
    this->setWindowFlags(Qt::Window);
    resize(600,600);
    setWindowTitle("3D View [Beta]");

    // Redraw when moving the camera
    //connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(updatePicking()));
    ////connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(updateHighlightedObject(int, int)));
    //connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(update()));

    ////connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(updatePicking()));
    ////connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(updateHighlightedObject(int, int)));
    connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(update()));

    //connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(updatePicking()));
    //connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(updateHighlightedObject(int, int)));
    connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(update()));

    //connect(global(),SIGNAL(keyboardModifiersChanged()),this,SLOT(handleNewKeyboardModifiers()));

    connect(global()->timeline(), SIGNAL(playingWindowChanged()), this, SLOT(update()));
}

View3D::~View3D()
{
    deletePicking();
}

View3DSettings * View3D::settings() {
    return &viewSettings_;
}

void View3D::closeEvent(QCloseEvent * event)
{
    emit closed();
    event->accept();
}

void View3D::keyPressEvent(QKeyEvent *event)
{
    GLWidget::keyPressEvent(event);

    // Nothing changed  in the scene, but keyboard  state can affect
    // the display settings, hence should redraw the scene. Pass the
    // event to multiview to do this
    if(!event->isAccepted())
        event->ignore();
}

      
View3D::MouseEvent View3D::mouseEvent() const
{
    MouseEvent me;
    me.x = mouse_PressEvent_X_;
    me.y = mouse_PressEvent_Y_;
    me.left = mouse_LeftButton_;
    me.mid = mouse_MidButton_;
    me.right = mouse_RightButton_;
    me.alt = mouse_AltWasDown_;
    me.control = mouse_ControlWasDown_;
    me.shift = mouse_ShiftWasDown_;
    return me;
}

void View3D::MoveEvent(double x, double y)
{
    bool hasChanged = updateHighlightedObject(x, y);
    if(hasChanged)
    {
        if(highlightedObject_.isNull())
            scene_->setNoHoveredObject();
        else
            scene_->setHoveredObject(
                //Timeline::time(highlightedObject_.time()),
                Time(), // ignored by VAC anyway...
                highlightedObject_.index(),
                highlightedObject_.id());
    }
}

int View3D::decideClicAction()
{
    if(mouse_LeftButton_)
    {
        if(!mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_)
        {
            if(highlightedObject_.isNull())
                return DESELECTALL_ACTION;
            else
                return SELECT_ACTION;
        }
        if(!mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           mouse_ShiftWasDown_)
        {
            return ADDSELECT_ACTION;
        }
        if(mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_)
        {
            return DESELECT_ACTION;
        }
        if(mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           mouse_ShiftWasDown_)
        {
            return TOGGLESELECT_ACTION;
        }
    }    
    return GLWidget::decideClicAction();
}

int View3D::decidePMRAction()
{
    return GLWidget::decidePMRAction();
}

void View3D::ClicEvent(int action, double x, double y)
{
    if(action==SELECT_ACTION)
    {
        if(!highlightedObject_.isNull())
        {
            scene_->deselectAll(); // deselect       at      all
                             // times. different from 2D behaviour
            scene_->select(Time() /*Timeline::time(highlightedObject_.time())*/,
                       highlightedObject_.index(),
                       highlightedObject_.id());
        }
    }
    else if(action==DESELECTALL_ACTION)
    {
        // same here: deselect at all times
        // different from 2D version
        scene_->deselectAll();
        
    }
    else if(action==ADDSELECT_ACTION)
    {
        if(!highlightedObject_.isNull())
        {
            scene_->select(Time()/*Timeline::time(highlightedObject_.time())*/,
                       highlightedObject_.index(),
                       highlightedObject_.id());
        }
    }
    else if(action==DESELECT_ACTION)
    {
        if(!highlightedObject_.isNull())
        {
            scene_->deselect(Time()/*Timeline::time(highlightedObject_.time())*/,
                         highlightedObject_.index(),
                         highlightedObject_.id());
        }
    }
    else if(action==TOGGLESELECT_ACTION)
    {
        if(!highlightedObject_.isNull())
        {
            scene_->toggle(Time()/*Timeline::time(highlightedObject_.time())*/,
                       highlightedObject_.index(),
                       highlightedObject_.id());
        }
    }
    GLWidget::ClicEvent(action, x, y);
}



void View3D::PMRPressEvent(int action, double x, double y)
{
    GLWidget::PMRPressEvent(action, x, y);
}

void View3D::PMRMoveEvent(int action, double x, double y)
{
    GLWidget::PMRMoveEvent(action, x, y);
}

void View3D::PMRReleaseEvent(int action, double x, double y)
{
    GLWidget::PMRReleaseEvent(action, x, y);
}

/***********************************************************
 *              DRAWING
 */

namespace
{
/*
void drawSphere(double r, int lats, int longs)
{
    int i, j;
    for(i = 0; i <= lats; i++)
    {
        double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
        double z0  = sin(lat0);
        double zr0 =  cos(lat0);

        double lat1 = M_PI * (-0.5 + (double) i / lats);
        double z1 = sin(lat1);
        double zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++)
        {
            double lng = 2 * M_PI * (double) (j - 1) / longs;
            double x = cos(lng);
            double y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
    }
}
*/
}

int View3D::activeFrame() const
{
    return std::floor(activeTime().floatTime());
}

Time View3D::activeTime() const
{
    return global()->activeTime(); // XXX should refactor this
}

void View3D::drawBackground_(Background * background, double t)
{
    // Get canvas boundary
    double x1 = scene_->left();
    double y1 = scene_->top();
    double w = scene_->width();
    double h = scene_->height();
    double x2 = x1 + w;
    double y2 = y1 + h;

    // Convert to 3D coords
    x1 = viewSettings_.xFromX2D(x1);
    x2 = viewSettings_.xFromX2D(x2);
    y1 = viewSettings_.yFromY2D(y1);
    y2 = viewSettings_.yFromY2D(y2);

    // Draw background
    backgroundRenderers_[background]->draw(
                Time(t).frame(),
                true, // = showCanvas
                x1, y1, w, h,
                0, 0, 0, 0);
}

// XXX Refactor this: move it to a CanvasRenderer class
// Right now, this codes duplicates part of Scene::drawCanvas()
void View3D::drawCanvas_()
{
    // Get canvas boundary
    double x1 = scene_->left();
    double y1 = scene_->top();
    double w = scene_->width();
    double h = scene_->height();
    double x2 = x1 + w;
    double y2 = y1 - h;

    // Convert to 3D coords
    x1 = viewSettings_.xFromX2D(x1);
    x2 = viewSettings_.xFromX2D(x2);
    y1 = viewSettings_.yFromY2D(y1);
    y2 = viewSettings_.yFromY2D(y2);

    // Draw quad boundary
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_QUADS);
    {
        glColor4f(0.0, 0.0, 0.0, 1.0);
        glVertex2d(x1, y1);
        glVertex2d(x2, y1);
        glVertex2d(x2, y2);
        glVertex2d(x1, y2);
    }
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}

void View3D::drawScene()
{
    using namespace VectorAnimationComplex;

    // Get VAC
    VAC * vac = scene_->activeVAC();
    if (!vac) {
        return;
    }

    // Get t-position of camera eye to determine back-to front order
    double zEye = camera_.position()[2];
    double tEye = - zEye / viewSettings_.timeScale();
    if(viewSettings_.cameraFollowActiveTime())
        tEye += activeTime().floatTime();

    // Scale and translate view
    glEnable(GL_NORMALIZE);
    double s = viewSettings_.spaceScale();
    glPushMatrix();
    glScaled(s,s,s);
    if(viewSettings_.cameraFollowActiveTime())
        glTranslated(0,0,-viewSettings_.zFromT(global()->activeTime()));


    // ----- Draw opaque objects first, with depth test enabled -----

    // Here, depth buffer writing is enabled by default

    // Disable lighting
    glDisable(GL_LIGHTING);

    // Draw inbetween cells
    if(viewSettings_.drawInbetweenCells())
        vac->drawInbetweenCells3D(viewSettings_);


    // ----- Then, draw transparent objects, back to front, with depth buffer writing disabled -----

    // Set 2D settings from 3D settings
    ViewSettings view2DSettings = global()->activeView()->viewSettings();
    view2DSettings.setScreenRelative(false);
    view2DSettings.setVertexTopologySize(viewSettings_.vertexTopologySize());
    view2DSettings.setEdgeTopologyWidth(viewSettings_.edgeTopologyWidth());
    view2DSettings.setDrawTopologyFaces(viewSettings_.drawTopologyFaces());

    // Disable writing to depth buffer
    glDepthMask(false);

    // Get the list of all ordered cells
    const ZOrderedCells & cells = vac->zOrdering();
    typedef ZOrderedCells::ConstIterator Iter;

    // Find what times to draw, and for each the following parameters:
    //    1. Should we draw no cells (i.e., just the canvas), only key cells, or all cells?
    //    2. Should we draw as topology or as illustration?
    //    3. should we draw canvas (+ background)?

    enum WhatCells { NoCells, KeyCells, AllCells };

    struct Params {
        WhatCells whatCellsToDraw;
        bool drawAsTopology;
        bool drawCanvas;
    };

    QMap<double, Params> timesToDraw;
    typedef QMapIterator<double, Params> MapIter;

    // Key cells
    if(viewSettings_.drawKeyCells())
    {
        // Params for drawing key cells
        Params params;
        params.whatCellsToDraw = KeyCells;
        params.drawAsTopology = viewSettings_.drawFramesAsTopology();
        params.drawCanvas = false;

        // Find times with key cells
        for(Iter it = cells.cbegin(); it != cells.cend(); ++it)
        {
            KeyCell * kc = (*it)->toKeyCell();
            if(kc)
                timesToDraw[kc->time().floatTime()] = params;
        }
    }

    // All frames
    if(viewSettings_.drawAllFrames())
    {
        // Params for drawing key cells
        Params params;
        params.whatCellsToDraw = AllCells;
        params.drawAsTopology = viewSettings_.drawFramesAsTopology();
        params.drawCanvas = false;

        // Find times for all cells
        Timeline * timeline = global()->timeline();
        int firstFrame = timeline->firstFrame();
        int lastFrame = timeline->lastFrame();
        for(int i=firstFrame; i<=lastFrame; ++i)
            timesToDraw[(double)i] = params;
    }

    // Current frame
    if(viewSettings_.drawTimePlane() || viewSettings_.drawCurrentFrame())
    {
        // Params for drawing key cells
        Params params;
        params.whatCellsToDraw = viewSettings_.drawCurrentFrame() ? AllCells : NoCells;
        params.drawAsTopology = viewSettings_.drawCurrentFrameAsTopology();
        params.drawCanvas = viewSettings_.drawTimePlane();

        // Add current time to list of times to draw
        timesToDraw[global()->activeTime().floatTime()] = params;
    }

    // Then, now that we have all times, find out in which order to draw them
    QList<double> timesBeforeEye;
    QList<double> timesAfterEye;
    QList<double> sortedTimes;
    MapIter i(timesToDraw);
    while (i.hasNext())
    {
        i.next();
        double t = i.key();
        if (t < tEye)
            timesBeforeEye << t;
        else
            timesAfterEye << t;
    }
    std::sort(timesBeforeEye.begin(), timesBeforeEye.end());
    std::sort(timesAfterEye.begin(), timesAfterEye.end());
    for (int i=0; i<timesBeforeEye.size(); ++i)
        sortedTimes << timesBeforeEye[i];
    for (int i=timesAfterEye.size()-1; i>=0; --i)
        sortedTimes << timesAfterEye[i];

    // Now, we "just" have to draw them!

    // Disable lighting
    glDisable(GL_LIGHTING);

    // Iterate times
    for (double t: sortedTimes)
    {
        // Get params for that time
        Params params = timesToDraw[t];

        // Translate to appropriate z value
        glPushMatrix();
        glScaled(1, -1, 1);
        glTranslated(0,0,viewSettings_.zFromT(t));

        // Draw canvas + background
        if(params.drawCanvas)
        {
            drawCanvas_();
            // XXX Make it work with layers
            //drawBackground_(scene_->background(), t);
        }

        // Draw cells
        if (params.whatCellsToDraw != NoCells)
        {
            if (params.drawAsTopology)
            {
                if (params.whatCellsToDraw == KeyCells)
                {
                    for(Iter it = cells.cbegin(); it != cells.cend(); ++it)
                    {
                        if ((*it)->toKeyCell())
                            (*it)->drawTopology(t, view2DSettings);
                    }
                }
                else
                {
                    for(Iter it = cells.cbegin(); it != cells.cend(); ++it)
                    {
                        (*it)->drawTopology(t, view2DSettings);
                    }
                }
            }
            else // params.drawAsTopology == false
            {
                if (params.whatCellsToDraw == KeyCells)
                {
                    for(Iter it = cells.cbegin(); it != cells.cend(); ++it)
                    {
                        if ((*it)->toKeyCell())
                            (*it)->draw(t, view2DSettings);
                    }
                }
                else
                {
                    for(Iter it = cells.cbegin(); it != cells.cend(); ++it)
                    {
                        (*it)->draw(t, view2DSettings);
                    }
                }
            }
        }

        // Translate back
        glPopMatrix();
    }

    // Restore state
    glDepthMask(true);
    glPopMatrix();
}


/***********************************************************
 *              PICKING
 */

void View3D::drawPick3D()
{
    if(scene_->activeVAC())
    {
        scene_->activeVAC()->drawPick3D(viewSettings_);
    }
}

bool View3D::updateHighlightedObject(int x, int y)
{
    if(!pickingImg_)
        return false; // otherwise the scene will keep updating

    Picking::Object old = highlightedObject_;
    if(x<0 || x>=pickingWidth_ || y<0 || y>=pickingHeight_)
    {
        highlightedObject_ = Picking::Object();
    }
    else
    {
        highlightedObject_ = getCloserObject(x, y);
    }
    return !(highlightedObject_ == old);
}

uchar * View3D::pickingImg(int x, int y)
{
    int k = 4*( (pickingHeight_ - y - 1)*pickingWidth_ + x);
    return &pickingImg_[k];
}

    
Picking::Object View3D::getCloserObject(int x, int y)
{
    Picking::Object noObject;
    
    if(!pickingImg_)
        return noObject; 

    int leftBorderDist = x;
    int rightBorderDist = pickingWidth_-1-x;
    int topBorderDist = y;
    int bottomBorderDist = pickingHeight_-1-y;

    int borderDist = qMin(qMin(leftBorderDist, rightBorderDist), 
                    qMin(topBorderDist, bottomBorderDist));

    if(borderDist<0)
        return noObject;

    int D = 10;
    if(borderDist < D)
        D = borderDist;
    
    for(int d=0; d<=D; d++)
    {
        if(d==0)
        {
            uchar * p = pickingImg(x,y);
            uchar r=p[0], g=p[1], b=p[2];
            if(r!=255 || g!=255 || b!=255)
                return Picking::objectFromRGB(r,g,b);
        }
        else
        {
            // top row
            for(int varX=x-d; varX<=x+d; varX++)
            {
                uchar * p = pickingImg(varX,y-d);
                uchar r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // bottom row
            for(int varX=x-d; varX<=x+d; varX++)
            {
                uchar * p = pickingImg(varX,y+d);
                uchar r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // left column
            for(int varY=y-d; varY<=y+d; varY++)
            {
                uchar * p = pickingImg(x-d,varY);
                uchar r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // right column
            for(int varY=y-d; varY<=y+d; varY++)
            {
                uchar * p = pickingImg(x+d,varY);
                uchar r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            
        }
    }

    // no object within the range
    return noObject;
}

void View3D::deletePicking()
{
    if(pickingImg_)
    {
        gl_fbo_->glDeleteFramebuffers(1, &fboId_);
        gl_fbo_->glDeleteRenderbuffers(1, &rboId_);
        glDeleteTextures(1, &textureId_);
        highlightedObject_ = Picking::Object();
        delete[] pickingImg_;
        pickingImg_ = 0;
        pickingWidth_ = 0;
        pickingHeight_ = 0;
    }
}

void View3D::newPicking()
{
    pickingWidth_ = width();
    pickingHeight_ = height();
    pickingImg_ = new uchar[4 * pickingWidth_ * pickingHeight_];

    //  code adapted from http://www.songho.ca/opengl/gl_fbo.html

    // create a texture object
    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pickingWidth_, pickingHeight_, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // create a renderbuffer object to store depth info
    gl_fbo_->glGenRenderbuffers(1, &rboId_);
    gl_fbo_->glBindRenderbuffer(GL_RENDERBUFFER, rboId_);
    gl_fbo_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                                   pickingWidth_, pickingHeight_);
    gl_fbo_->glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // create a framebuffer object
    gl_fbo_->glGenFramebuffers(1, &fboId_);
    gl_fbo_->glBindFramebuffer(GL_FRAMEBUFFER, fboId_);

    // attach the texture to FBO color attachment point
    gl_fbo_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                    GL_TEXTURE_2D, textureId_, 0);

    // attach the renderbuffer to depth attachment point
    gl_fbo_->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                       GL_RENDERBUFFER, rboId_);

    // check FBO status
    GLenum status = gl_fbo_->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        qDebug() << "ERROR void View::newPicking()"
               << "FBO status != GL_FRAMEBUFFER_COMPLETE";
        return;
    }

    // switch back to window-system-provided framebuffer
    gl_fbo_->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
}


void View3D::updatePicking()
{
    // Make this widget's rendering context the current OpenGL context
    makeCurrent();

    // get the viewport size, allocate memory if necessary
    if( !(width()>0) || !(height()>0))
    {
        deletePicking();
        return;
    }
    else if(
        pickingImg_
        && (pickingWidth_ == width())
        && (pickingHeight_ == height()))
    {
        // necessary objects already created: do nothing
    }
    else
    {
        deletePicking();
        newPicking();
    }

    // set rendering destination to FBO
    gl_fbo_->glBindFramebuffer(GL_FRAMEBUFFER, fboId_);

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // clear buffers
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Should we setup other things? (e.g., disabling antialiasing)
    // Seems to work as is. If issues, check GLWidget::initilizeGL()

    // Set viewport
    GLint oldViewport[4];
    glGetIntegerv(GL_VIEWPORT, oldViewport);
    glViewport(0, 0, pickingWidth_, pickingHeight_);

    // Setup camera position and orientation
    setCameraPositionAndOrientation();

    // draw the picking
    drawPick3D();

    // Restore viewport
    glViewport(oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);

    // unbind FBO
    gl_fbo_->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // extract the texture info from GPU to RAM
    glBindTexture(GL_TEXTURE_2D, textureId_); 
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pickingImg_);
    glBindTexture(GL_TEXTURE_2D, 0);
}
