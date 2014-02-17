/*
 * Copyright (C) 2014 Milian Wolff, KDAB (milian.wolff@kdab.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef QWEBFRAME_PRINTINGADDONS_P_H
#define QWEBFRAME_PRINTINGADDONS_P_H

#include "qwebframe.h"
#include "qwebframe_p.h"

#include <qprinter.h>
#include <qstring.h>

#include "GraphicsContext.h"
#include "QtPrintContext.h"

// for custom header or footers in printing
class HeaderFooter
{
public:
    HeaderFooter(QPrinter* printer, QWebFrame::PrintCallback* callback);
    ~HeaderFooter();

    void paintHeader(QtPrintContext* ctx, const WebCore::IntRect& pageRect, int pageNum, int totalPages);
    void paintFooter(QtPrintContext* ctx, const WebCore::IntRect& pageRect, int pageNum, int totalPages);

    void paint(int page, const QRect& pageRect, QtPrintContext* printContext);

private:
    QWebPage page;
    QWebFrame::PrintCallback* callback;
    int headerHeightPixel;
    int footerHeightPixel;

    void paint(QtPrintContext* ctx, int offset, const QRect& pageRect, const QString& contents, int height);
};

HeaderFooter::HeaderFooter(QPrinter* printer, QWebFrame::PrintCallback* callback_)
: callback(callback_)
, headerHeightPixel(0)
, footerHeightPixel(0)
{
    if (callback) {
        qreal headerHeight = qMax(qreal(0), callback->headerHeight());
        qreal footerHeight = qMax(qreal(0), callback->footerHeight());

        if (headerHeight || footerHeight) {
            // figure out the header/footer height in *DevicePixel*
            // based on the height given in *Points*
            qreal marginLeft, marginRight, marginTop, marginBottom;
            // find existing margins
            printer->getPageMargins(&marginLeft, &marginTop, &marginRight, &marginBottom, QPrinter::DevicePixel);
            const qreal oldMarginTop = marginTop;
            const qreal oldMarginBottom = marginBottom;

            printer->getPageMargins(&marginLeft, &marginTop, &marginRight, &marginBottom, QPrinter::Point);
            // increase margins to hold header+footer
            marginTop += headerHeight;
            marginBottom += footerHeight;
            printer->setPageMargins(marginLeft, marginTop, marginRight, marginBottom, QPrinter::Point);

            // calculate actual heights
            printer->getPageMargins(&marginLeft, &marginTop, &marginRight, &marginBottom, QPrinter::DevicePixel);
            headerHeightPixel = marginTop - oldMarginTop;
            footerHeightPixel = marginBottom - oldMarginBottom;
        }
    }
}

HeaderFooter::~HeaderFooter()
{
}

void HeaderFooter::paint(int page, const QRect& pageRect, QtPrintContext* printContext)
{
    if (!callback || (headerHeightPixel <= 0 && footerHeightPixel <= 0)) {
        return;
    }

    int logicalPage, logicalPages;
    printContext->getPagination(page, logicalPage, logicalPages);

    paint(printContext, -headerHeightPixel, pageRect, callback->header(logicalPage, logicalPages), headerHeightPixel);
    paint(printContext, pageRect.height(), pageRect, callback->footer(logicalPage, logicalPages), footerHeightPixel);
}

void HeaderFooter::paint(QtPrintContext* ctx, int offset, const QRect& pageRect, const QString& contents, int height)
{
    if (contents.isEmpty() || !height) {
        return;
    }

    page.mainFrame()->setHtml(contents);
    ctx->paintHeaderFooter(QWebFramePrivate::webcoreFrame(page.mainFrame()), offset, pageRect, height);
}


#endif // QWEBFRAME_PRINTINGADDONS_P_H
