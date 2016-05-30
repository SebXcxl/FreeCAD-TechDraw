/***************************************************************************
 *   Copyright (c) Ian Rees                    (ian.rees@gmail.com) 2015   *
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

#ifndef DRAWINGGUI_TEMPLATETEXTFIELD_H
#define DRAWINGGUI_TEMPLATETEXTFIELD_H

#include <QGraphicsRectItem>

#include "../App/DrawTemplate.h"

QT_BEGIN_NAMESPACE
class QGI;
QT_END_NAMESPACE

namespace TechDrawGui
{
    /// QGraphicsRectItem-derived class for the text fields in title blocks
    /*!
     * Makes a rectangular area which can be clicked to open up a text editing
     * dialog.  Changes an appropriate Property in the Drawing's template.
     */
    class TechDrawGuiExport TemplateTextField : public QGraphicsRectItem
    {
        public:
            TemplateTextField(QGraphicsItem*parent,
                              TechDraw::DrawTemplate *myTmplte,
                              const std::string &myFieldName);

            ~TemplateTextField() = default;

            enum {Type = QGraphicsItem::UserType + 160};
            int type() const { return Type;}

            /// Returns the field name that this TemplateTextField represents
            std::string fieldName() const { return fieldNameStr; }

        protected:
            virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
            TechDraw::DrawTemplate *tmplte;
            std::string fieldNameStr;
    };
}   // namespace TechDrawGui

#endif // #ifndef DRAWINGGUI_TEMPLATETEXTFIELD_H
