/***************************************************************************
 *   Copyright (c) 2013 Luke Parry <l.parry@warwick.ac.uk>                 *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#include "PreCompiled.h"
#ifndef _PreComp_
# include <QAction>
# include <QApplication>
# include <QContextMenuEvent>
# include <QFileInfo>
# include <QFileDialog>
# include <QGLWidget>
# include <QGraphicsScene>
# include <QGraphicsEffect>
# include <QMouseEvent>
# include <QPainter>
# include <QPaintEvent>
# include <QWheelEvent>
# include <strstream>
# include <cmath>
#endif

#include "Base/Console.h"
#include "Base/Stream.h"
#include "Gui/FileDialog.h"
#include "Gui/WaitCursor.h"

#include "../App/Geometry.h"
#include "../App/DrawTemplate.h"
#include "../App/DrawSVGTemplate.h"
#include "../App/DrawParametricTemplate.h"
#include "../App/DrawViewCollection.h"
#include "../App/DrawViewDimension.h"
#include "../App/DrawProjGroup.h"
#include "../App/DrawViewSection.h"
#include "../App/DrawViewPart.h"
#include "../App/DrawViewAnnotation.h"
#include "../App/DrawViewSymbol.h"
#include "../App/DrawViewClip.h"
#include "../App/DrawHatch.h"
#include "../App/DrawViewSpreadsheet.h"

#include "ViewProviderPage.h"

#include "QGISVGTemplate.h"
#include "QGIViewCollection.h"
#include "QGIViewDimension.h"
#include "QGIProjGroup.h"
#include "QGIViewPart.h"
#include "QGIViewSection.h"
#include "QGIViewAnnotation.h"
#include "QGIViewSymbol.h"
#include "QGIViewClip.h"
#include "QGIViewSpreadsheet.h"

#include "../App/GraphicsItems/GIDrawingTemplate.h"
#include "../App/GraphicsItems/ZVALUE.h"

#include "QGVPage.h"

using namespace TechDrawGui;

QGVPage::QGVPage(TechDraw::DrawPage *page, QWidget *parent)
    : QGraphicsView(parent),
      GIPage(page, parent),
      m_renderer(Native),
      drawBkg(true),
      m_backgroundItem(0),
      m_outlineItem(0)
{
    setScene(m_scene);

    const char *name = m_page->getNameInDocument();
    setObjectName(QString::fromLocal8Bit(name));

    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setCacheMode(QGraphicsView::CacheBackground);
    setTransformationAnchor(AnchorUnderMouse);

    setDragMode(ScrollHandDrag);
    setCursor(QCursor(Qt::ArrowCursor));
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    m_backgroundItem = new QGraphicsRectItem();
    m_backgroundItem->setCacheMode(QGraphicsItem::NoCache);
    m_backgroundItem->setZValue(ZVALUE::BACKGROUND);
//     scene()->addItem(m_backgroundItem); // TODO IF SEGFAULTS WITH DRAW ENABLE THIS (REDRAWS ARE SLOWER :s)

    // Prepare background check-board pattern
    QLinearGradient gradient;
    gradient.setStart(0, 0);
    gradient.setFinalStop(0, height());
    gradient.setColorAt(0., QColor(72, 72, 72));
    gradient.setColorAt(1., QColor(150, 150, 150));
    bkgBrush = new QBrush(QColor::fromRgb(70,70,70));

    resetCachedContent();
}

QGVPage::~QGVPage()
{
    delete bkgBrush;
    delete m_backgroundItem;
}

void QGVPage::drawBackground(QPainter *p, const QRectF &)
{
    if(!drawBkg)
        return;

    p->save();
    p->resetTransform();

    p->setBrush(*bkgBrush);
    p->drawRect(viewport()->rect());

    // Default to A3 landscape, though this is currently relevant
    // only for opening corrupt docs, etc.
    // TODO: Move this higher up - probably never relevant, but ugly to have constants this low down
    float pageWidth = 420,
          pageHeight = 297;

    if ( m_page->hasValidTemplate() ) {
        pageWidth = m_page->getPageWidth();
        pageHeight = m_page->getPageHeight();
    }

    // Draw the white page
    QRectF paperRect(0, -pageHeight, pageWidth, pageHeight);
    QPolygon poly = mapFromScene(paperRect);

    QBrush pageBrush(Qt::white);
    p->setBrush(pageBrush);

    p->drawRect(poly.boundingRect());

    p->restore();
}

QGIView * QGVPage::addViewPart(TechDraw::DrawViewPart *part)
{
    QGIViewPart *viewPart(new QGIViewPart);

    viewPart->setViewPartFeature(part);

    addView(viewPart);
    return viewPart;
}

QGIView * QGVPage::addViewSection(TechDraw::DrawViewPart *part)
{
    QGIViewSection *viewSection(new QGIViewSection);
    viewSection->setViewPartFeature(part);

    addView(viewSection);
    return viewSection;
}

QGIView * QGVPage::addProjectionGroup(TechDraw::DrawProjGroup *view) {
    QGIViewCollection *qview(new QGIProjGroup);

    qview->setViewFeature(view);
    addView(qview);
    return qview;
}

QGIView * QGVPage::addDrawView(TechDraw::DrawView *view)
{
    QGIView *qview(new QGIView);

    qview->setViewFeature(view);
    addView(qview);

    return qview;
}

QGIView * QGVPage::addDrawViewCollection(TechDraw::DrawViewCollection *view)
{
    QGIViewCollection *qview(new QGIViewCollection);

    qview->setViewFeature(view);
    addView(qview);
    return qview;
}

// TODO change to (App?) annotation object  ??
QGIView * QGVPage::addDrawViewAnnotation(TechDraw::DrawViewAnnotation *view)
{
    // This essentially adds a null view feature to ensure view size is consistent
    QGIViewAnnotation *qview(new QGIViewAnnotation);

    qview->setViewAnnoFeature(view);

    addView(qview);
    return qview;
}

QGIView * QGVPage::addDrawViewSymbol(TechDraw::DrawViewSymbol *view)
{
    // This essentially adds a null view feature to ensure view size is consistent
    QGIViewSymbol *qview(new QGIViewSymbol);
    qview->setViewFeature(view);

    addView(qview);
    return qview;
}

QGIView * QGVPage::addDrawViewClip(TechDraw::DrawViewClip *view)
{
    QPoint qp(view->X.getValue(),view->Y.getValue());
    QGIViewClip *qview(new QGIViewClip);
    qview->setViewFeature(view);

    addView(qview);
    return qview;
}

QGIView * QGVPage::addDrawViewSpreadsheet(TechDraw::DrawViewSpreadsheet *view)
{
    QGIViewSpreadsheet *qview(new QGIViewSpreadsheet);

    qview->setViewFeature(view);

    addView(qview);
    return qview;
}

int QGVPage::addView(TechDraw::GIBase *view)
{
    auto scn(scene());
    assert(scn);
    scn->addItem(view);

    return TechDraw::GIPage::addView(view);
}

QGIView * QGVPage::addViewDimension(TechDraw::DrawViewDimension *dim)
{
    QGIViewDimension *dimGroup(new QGIViewDimension() );

    auto sc(scene());
    assert(sc);
    sc->addItem(dimGroup);

    dimGroup->setViewPartFeature(dim);

    // TODO consider changing dimension feature to use another property for label position
    // Instead of calling addView - the view must for now be added manually

    //Note dimension X,Y is different from other views -> can't use addView
    views.push_back(dimGroup);

    // Find if it belongs to a parent
    auto parent( dynamic_cast<QGIView *>(findParent(dimGroup)) );

    if(parent) {
        addDimToParent(dimGroup, parent);
    }

    return dimGroup;
}

void QGVPage::addDimToParent(QGIViewDimension* dim, QGIView* parent)
{
    assert(dim);
    assert(parent);          //blow up if we don't have Dimension or Parent
    QPointF posRef(0.,0.);
    QPointF mapPos = dim->mapToItem(parent, posRef);
    dim->moveBy(-mapPos.x(), -mapPos.y());
    parent->addToGroup(dim);
    dim->setZValue(ZVALUE::DIMENSION);
}


void QGVPage::setPageFeature(TechDraw::DrawPage *page)
{
    //redundant
#if 0
    // TODO verify if the pointer should even be used. Not really safe
    pageFeat = page;

    float pageWidth  = m_page->getPageWidth();
    float pageHeight = m_page->getPageHeight();

    QRectF paperRect(0, -pageHeight, pageWidth, pageHeight);

    QBrush brush(Qt::white);

    m_backgroundItem->setBrush(brush);
    m_backgroundItem->setRect(paperRect);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10.0);
    shadow->setColor(Qt::white);
    shadow->setOffset(0,0);
    m_backgroundItem->setGraphicsEffect(shadow);

    QRectF myRect = paperRect;
    myRect.adjust(-20,-20,20,20);
    setSceneRect(myRect);
#endif
}


void QGVPage::setRenderer(RendererType type)
{
    m_renderer = type;

    if (m_renderer == OpenGL) {
#ifndef QT_NO_OPENGL
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
#endif
    } else {
        setViewport(new QWidget);
    }
}

void QGVPage::setHighQualityAntialiasing(bool highQualityAntialiasing)
{
#ifndef QT_NO_OPENGL
    setRenderHint(QPainter::HighQualityAntialiasing, highQualityAntialiasing);
#else
    Q_UNUSED(highQualityAntialiasing);
#endif
}

void QGVPage::setViewBackground(bool enable)
{
    if (!m_backgroundItem)
        return;

    m_backgroundItem->setVisible(enable);
}

void QGVPage::setViewOutline(bool enable)
{
    if (!m_outlineItem)
        return;

    m_outlineItem->setVisible(enable);
}

void QGVPage::toggleEdit(bool enable)
{
// TODO: needs fiddling to handle items that don't inherit QGIViewPart: Annotation, Symbol, Templates, Edges, Faces, Vertices,...
    QList<QGraphicsItem*> list = scene()->items();

    for (QList<QGraphicsItem*>::iterator it = list.begin(); it != list.end(); ++it) {
        QGIView *itemView = dynamic_cast<QGIView *>(*it);
        if(itemView) {
            QGIViewPart *viewPart = dynamic_cast<QGIViewPart *>(*it);
            itemView->setSelected(false);
            if(viewPart) {
                viewPart->toggleCache(enable);
                viewPart->toggleCosmeticLines(enable);
                viewPart->toggleVertices(enable);
                viewPart->toggleBorder(enable);
                setViewBackground(enable);
            } else {
                itemView->toggleBorder(enable);
            }
            //itemView->updateView(true);
        }

        int textItemType = QGraphicsItem::UserType + 160;
        QGraphicsItem*item = dynamic_cast<QGraphicsItem*>(*it);
        if(item) {
            //item->setCacheMode((enable) ? QGraphicsItem::DeviceCoordinateCache : QGraphicsItem::NoCache);
            item->setCacheMode((enable) ? QGraphicsItem::NoCache : QGraphicsItem::NoCache);
            item->update();
            if (item->type() == textItemType) {    //TODO: move this into SVGTemplate or TemplateTextField
                if (enable) {
                    item->show();
                } else {
                    item->hide();
                }
            }
        }
    }
    scene()->update();
    update();
    viewport()->repaint();
}

void QGVPage::saveSvg(QString filename)
{
    // fiddle cache, cosmetic lines, vertices, etc
    Gui::Selection().clearSelection();
    toggleEdit(false);

    GIPage::saveSvg(filename);

    toggleEdit(true);

}

void QGVPage::paintEvent(QPaintEvent *event)
{
    if (m_renderer == Image) {
        if (m_image.size() != viewport()->size()) {
            m_image = QImage(viewport()->size(), QImage::Format_ARGB32_Premultiplied);
        }

        QPainter imagePainter(&m_image);
        QGraphicsView::render(&imagePainter);
        imagePainter.end();

        QPainter p(viewport());
        p.drawImage(0, 0, m_image);

    } else {
        QGraphicsView::paintEvent(event);
    }
}

void QGVPage::wheelEvent(QWheelEvent *event)
{
    qreal factor = std::pow(1.2, -event->delta() / 240.0);
    scale(factor, factor);
    event->accept();
}
void QGVPage::enterEvent(QEvent *event)
{
    QGraphicsView::enterEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
}

void QGVPage::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    viewport()->setCursor(Qt::ClosedHandCursor);
}

void QGVPage::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::ArrowCursor);
}

int QGVPage::attachView(App::DocumentObject *obj)
{
    QGIView *qview(nullptr);

    auto typeId(obj->getTypeId());

    if(typeId.isDerivedFrom(TechDraw::DrawViewSection::getClassTypeId()) ) {
        qview = addViewSection( dynamic_cast<TechDraw::DrawViewSection *>(obj) );
    } else if (typeId.isDerivedFrom(TechDraw::DrawViewPart::getClassTypeId()) ) {
        qview = addViewPart( dynamic_cast<TechDraw::DrawViewPart *>(obj) );
    } else if (typeId.isDerivedFrom(TechDraw::DrawProjGroup::getClassTypeId()) ) {
        qview = addProjectionGroup( dynamic_cast<TechDraw::DrawProjGroup *>(obj) );
    } else if (typeId.isDerivedFrom(TechDraw::DrawViewCollection::getClassTypeId()) ) {
        qview = addDrawView( dynamic_cast<TechDraw::DrawViewCollection *>(obj) );
    } else if(typeId.isDerivedFrom(TechDraw::DrawViewDimension::getClassTypeId()) ) {
        qview = addViewDimension( dynamic_cast<TechDraw::DrawViewDimension *>(obj) );
    } else if(typeId.isDerivedFrom(TechDraw::DrawViewAnnotation::getClassTypeId()) ) {
        qview = addDrawViewAnnotation( dynamic_cast<TechDraw::DrawViewAnnotation *>(obj) );
    } else if(typeId.isDerivedFrom(TechDraw::DrawViewSymbol::getClassTypeId()) ) {
        qview = addDrawViewSymbol( dynamic_cast<TechDraw::DrawViewSymbol *>(obj) );
    } else if(typeId.isDerivedFrom(TechDraw::DrawViewClip::getClassTypeId()) ) {
        qview = addDrawViewClip( dynamic_cast<TechDraw::DrawViewClip *>(obj) );
    } else if(typeId.isDerivedFrom(TechDraw::DrawViewSpreadsheet::getClassTypeId()) ) {
        qview = addDrawViewSpreadsheet( dynamic_cast<TechDraw::DrawViewSpreadsheet *>(obj) );
    } else if(typeId.isDerivedFrom(TechDraw::DrawHatch::getClassTypeId()) ) {
        //Hatch is not attached like other Views (since it isn't really a View)
    } else {
        Base::Console().Log("Logic Error - Unknown view type in QGVPage::attachView()");
        assert(0);
    }

    if(!qview)
        return -1;
    else
        return views.size();
}

void QGVPage::attachTemplate(TechDraw::DrawTemplate *obj)
{
    // TODO: Assign a base template class and create object dependent on
    removeTemplate();

    if(obj->isDerivedFrom(TechDraw::DrawParametricTemplate::getClassTypeId())) {
        pageTemplate.reset( new TechDraw::GIDrawingTemplate );
    } else if(obj->isDerivedFrom(TechDraw::DrawSVGTemplate::getClassTypeId())) {
        pageTemplate.reset( new QGISVGTemplate );
    }

    pageTemplate->setTemplate(obj);
    pageTemplate->updateView();
    scene()->addItem(pageTemplate.get());

    setSceneLimits();
}

#include "moc_QGVPage.cpp"
