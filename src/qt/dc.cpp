/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/dc.cpp
// Author:      Peter Most
// Id:          $Id$
// Copyright:   (c) Peter Most, Javier Torres
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/dc.h"
#include "wx/qt/dc.h"
#include "wx/qt/converter.h"
#include "wx/qt/utils.h"
#include <QtGui/QBitmap>

wxQtDCImpl::wxQtDCImpl( wxDC *owner )
    : wxDCImpl( owner )
{
    m_clippingRegion = new wxRegion;
}

wxQtDCImpl::~wxQtDCImpl()
{
    if ( m_qtPainter.isActive() )
        m_qtPainter.end();

    if ( m_clippingRegion != NULL )
        delete m_clippingRegion;
}

void wxQtDCImpl::PrepareQPainter()
{
    //Do here all QPainter initialization (called after each begin())
}

bool wxQtDCImpl::CanDrawBitmap() const
{
    return false;
}

bool wxQtDCImpl::CanGetTextExtent() const
{
    return false;
}

void wxQtDCImpl::DoGetSize(int *width, int *height) const
{
    *width  = m_qtPainter.device()->width();
    *height = m_qtPainter.device()->height();
}

void wxQtDCImpl::DoGetSizeMM(int* width, int* height) const
{
    *width  = m_qtPainter.device()->widthMM();
    *height = m_qtPainter.device()->heightMM();
}

int wxQtDCImpl::GetDepth() const
{
    return m_qtPainter.device()->depth();
}

wxSize wxQtDCImpl::GetPPI() const
{
    return wxSize(m_qtPainter.device()->logicalDpiX(), m_qtPainter.device()->logicalDpiY());
}

void wxQtDCImpl::SetFont(const wxFont& font)
{
    m_qtPainter.setFont(font.GetHandle());
}

void wxQtDCImpl::SetPen(const wxPen& pen)
{
    m_qtPainter.setPen(pen.GetHandle());
}

void wxQtDCImpl::SetBrush(const wxBrush& brush)
{
    if (brush.GetStyle() == wxBRUSHSTYLE_STIPPLE_MASK_OPAQUE)
    {
        // Use a monochrome mask: use foreground color for the mask
        QBrush b(brush.GetHandle());
        b.setColor(m_textForegroundColour.GetHandle());
        b.setTexture(b.texture().mask());
        m_qtPainter.setBrush(b);
    }
    else if (brush.GetStyle() == wxBRUSHSTYLE_STIPPLE)
    {
        //Don't use the mask
        QBrush b(brush.GetHandle());

        QPixmap p = b.texture();
        p.setMask(QBitmap());
        b.setTexture(p);

        m_qtPainter.setBrush(b);
    }
    else
    {
        m_qtPainter.setBrush(brush.GetHandle());
    }
}

void wxQtDCImpl::SetBackground(const wxBrush& brush)
{
    m_qtPainter.setBackground(brush.GetHandle());
}

void wxQtDCImpl::SetBackgroundMode(int mode)
{
    /* Do not change QPainter, as wx uses this background mode
     * only for drawing text, where Qt uses it for everything.
     * Always let QPainter mode to transparent, and change it
     * when needed */
    m_backgroundMode = mode;
}


#if wxUSE_PALETTE
void wxQtDCImpl::SetPalette(const wxPalette& palette)
{
    wxMISSING_IMPLEMENTATION(__FUNCTION__);
}
#endif // wxUSE_PALETTE

void wxQtDCImpl::SetLogicalFunction(wxRasterOperationMode function)
{
    wxMISSING_IMPLEMENTATION(__FUNCTION__);
}


wxCoord wxQtDCImpl::GetCharHeight() const
{
    QFontMetrics metrics(m_qtPainter.font());
    return wxCoord( metrics.height() );
}

wxCoord wxQtDCImpl::GetCharWidth() const
{
    //FIXME: Returning max width, instead of average
    QFontMetrics metrics(m_qtPainter.font());
    return wxCoord( metrics.maxWidth() );
}

void wxQtDCImpl::DoGetTextExtent(const wxString& string,
                             wxCoord *x, wxCoord *y,
                             wxCoord *descent,
                             wxCoord *externalLeading,
                             const wxFont *theFont ) const
{
    QFont f = m_qtPainter.font();
    if (theFont != NULL)
        f = theFont->GetHandle();

    QFontMetrics metrics(f);
    if (x != NULL || y != NULL)
    {
        QRect bounding = metrics.boundingRect( wxQtConvertString(string) );

        if (x != NULL)
            *x = bounding.width();
        if (y != NULL)
            *y = bounding.height();
    }

    if (descent != NULL)
        *descent = metrics.descent();

    wxMISSING_IMPLEMENTATION( "DoGetTextExtent: externalLeading" );
    if (externalLeading != NULL)
        *externalLeading = 0;
}

void wxQtDCImpl::Clear()
{
    int width, height;
    DoGetSize(&width, &height);
    
    m_qtPainter.eraseRect(QRect(0, 0, width, height));
}

void wxQtDCImpl::DoSetClippingRegion(wxCoord x, wxCoord y,
                                 wxCoord width, wxCoord height)
{
    // Special case: Empty region -> DestroyClippingRegion()
    if ( width == 0 && height == 0 )
    {
        DestroyClippingRegion();
    }
    else
    {
        // Set QPainter clipping (intersection if not the first one)
        m_qtPainter.setClipRect( x, y, width, height,
                                 m_clipping ? Qt::IntersectClip : Qt::ReplaceClip );

        // Set internal state for getters
        /* Note: Qt states that QPainter::clipRegion() may be slow, so we
         * keep the region manually, which should be faster */
        if ( m_clipping )
            m_clippingRegion->Union( wxRect( x, y, width, height ) );
        else
            m_clippingRegion->Intersect( wxRect( x, y, width, height ) );

        wxRect clipRect = m_clippingRegion->GetBox();

        m_clipX1 = clipRect.GetLeft();
        m_clipX2 = clipRect.GetRight();
        m_clipY1 = clipRect.GetTop();
        m_clipY2 = clipRect.GetBottom();
        m_clipping = true;
    }
}

void wxQtDCImpl::DoSetDeviceClippingRegion(const wxRegion& region)
{
    if ( region.IsEmpty() )
    {
        DestroyClippingRegion();
    }
    else
    {
        // Set QPainter clipping (intersection if not the first one)
        m_qtPainter.setClipRegion( region.GetHandle(), 
                                 m_clipping ? Qt::IntersectClip : Qt::ReplaceClip );
                                 
        // Set internal state for getters
        /* Note: Qt states that QPainter::clipRegion() may be slow, so we
        * keep the region manually, which should be faster */
        if ( m_clipping )
            m_clippingRegion->Union( region );
        else
            m_clippingRegion->Intersect( region );

        wxRect clipRect = m_clippingRegion->GetBox();

        m_clipX1 = clipRect.GetLeft();
        m_clipX2 = clipRect.GetRight();
        m_clipY1 = clipRect.GetTop();
        m_clipY2 = clipRect.GetBottom();
        m_clipping = true;
    }
}

void wxQtDCImpl::DestroyClippingRegion()
{
    ResetClipping();
    m_clippingRegion->Clear();
    m_qtPainter.setClipping( false );
}

bool wxQtDCImpl::DoFloodFill(wxCoord x, wxCoord y, const wxColour& col,
                         wxFloodFillStyle style )
{
    m_qtPainter.device()->logicalDpiX();
    return false;
}

bool wxQtDCImpl::DoGetPixel(wxCoord x, wxCoord y, wxColour *col) const
{
    m_qtPainter.device()->logicalDpiX();
    return false;
}

void wxQtDCImpl::DoDrawPoint(wxCoord x, wxCoord y)
{
    m_qtPainter.drawPoint(x, y);
}

void wxQtDCImpl::DoDrawLine(wxCoord x1, wxCoord y1, wxCoord x2, wxCoord y2)
{
    m_qtPainter.drawLine(x1, y1, x2, y2);
}


void wxQtDCImpl::DoDrawArc(wxCoord x1, wxCoord y1,
                       wxCoord x2, wxCoord y2,
                       wxCoord xc, wxCoord yc)
{
    wxMISSING_IMPLEMENTATION(__FUNCTION__);
}

void wxQtDCImpl::DoDrawEllipticArc(wxCoord x, wxCoord y, wxCoord w, wxCoord h,
                               double sa, double ea)
{
    m_qtPainter.device()->logicalDpiX();
}

void wxQtDCImpl::DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
    m_qtPainter.drawRect(x, y, width, height);
}

void wxQtDCImpl::DoDrawRoundedRectangle(wxCoord x, wxCoord y,
                                    wxCoord width, wxCoord height,
                                    double radius)
{
    m_qtPainter.drawRoundedRect(x, y, width, height, radius, radius);
}

void wxQtDCImpl::DoDrawEllipse(wxCoord x, wxCoord y,
                           wxCoord width, wxCoord height)
{
    m_qtPainter.drawEllipse(x, y, width, height);
}

void wxQtDCImpl::DoCrossHair(wxCoord x, wxCoord y)
{
    wxMISSING_IMPLEMENTATION(__FUNCTION__);
}

void wxQtDCImpl::DoDrawIcon(const wxIcon& icon, wxCoord x, wxCoord y)
{
    wxMISSING_IMPLEMENTATION(__FUNCTION__);
}

void wxQtDCImpl::DoDrawBitmap(const wxBitmap &bmp, wxCoord x, wxCoord y,
                          bool useMask )
{
    //TODO: Don't use mask if useMask is false

    QPixmap pix = *bmp.GetHandle();
    if (pix.depth() == 1) {
        //Monochrome bitmap, draw using text fore/background
        
        //Save pen/brush
        QBrush savedBrush = m_qtPainter.background();
        QPen savedPen = m_qtPainter.pen();       
        
        //Use text colors
        m_qtPainter.setBackground(QBrush(m_textBackgroundColour.GetHandle()));
        m_qtPainter.setPen(QPen(m_textForegroundColour.GetHandle()));

        //Draw
        m_qtPainter.drawPixmap(x, y, pix);
        
        //Restore saved settings
        m_qtPainter.setBackground(savedBrush);
        m_qtPainter.setPen(savedPen);
    } else {
        m_qtPainter.drawPixmap(x, y, pix);
    }
}

void wxQtDCImpl::DoDrawText(const wxString& text, wxCoord x, wxCoord y)
{
    QPen savedPen = m_qtPainter.pen();
    m_qtPainter.setPen(QPen(m_textForegroundColour.GetHandle()));
    
    if (m_backgroundMode == wxSOLID)
    {
        m_qtPainter.setBackgroundMode(Qt::OpaqueMode);
    
        //Save pen/brush
        QBrush savedBrush = m_qtPainter.background();

        //Use text colors
        m_qtPainter.setBackground(QBrush(m_textBackgroundColour.GetHandle()));

        //Draw
        m_qtPainter.drawText(x, y, 1, 1, Qt::TextDontClip, wxQtConvertString(text));

        //Restore saved settings
        m_qtPainter.setBackground(savedBrush);


        m_qtPainter.setBackgroundMode(Qt::TransparentMode);
    }
    else
        m_qtPainter.drawText(x, y, 1, 1, Qt::TextDontClip, wxQtConvertString(text));

    m_qtPainter.setPen(savedPen);
}

void wxQtDCImpl::DoDrawRotatedText(const wxString& text,
                               wxCoord x, wxCoord y, double angle)
{
    if (m_backgroundMode == wxSOLID)
        m_qtPainter.setBackgroundMode(Qt::OpaqueMode);
    
    //Move and rotate (reverse angle direction in Qt and wx)
    m_qtPainter.translate(x, y);
    m_qtPainter.rotate(-angle);

    QPen savedPen = m_qtPainter.pen();
    m_qtPainter.setPen(QPen(m_textForegroundColour.GetHandle()));

    if (m_backgroundMode == wxSOLID)
    {
        m_qtPainter.setBackgroundMode(Qt::OpaqueMode);
        
        //Save pen/brush
        QBrush savedBrush = m_qtPainter.background();
        
        //Use text colors
        m_qtPainter.setBackground(QBrush(m_textBackgroundColour.GetHandle()));
        
        //Draw
        m_qtPainter.drawText(x, y, 1, 1, Qt::TextDontClip, wxQtConvertString(text));
        
        //Restore saved settings
        m_qtPainter.setBackground(savedBrush);
        
        m_qtPainter.setBackgroundMode(Qt::TransparentMode);
    }
    else
        m_qtPainter.drawText(x, y, 1, 1, Qt::TextDontClip, wxQtConvertString(text));

    //Reset to default
    ComputeScaleAndOrigin();
    m_qtPainter.setPen(savedPen);
}

bool wxQtDCImpl::DoBlit(wxCoord xdest, wxCoord ydest,
                    wxCoord width, wxCoord height,
                    wxDC *source,
                    wxCoord xsrc, wxCoord ysrc,
                    wxRasterOperationMode rop,
                    bool useMask,
                    wxCoord xsrcMask,
                    wxCoord ysrcMask )
{
    wxMISSING_IMPLEMENTATION(__FUNCTION__);
    return false;
}

void wxQtDCImpl::DoDrawLines(int n, wxPoint points[],
                         wxCoord xoffset, wxCoord yoffset )
{
    if (n > 0)
    {
        QPainterPath path(wxQtConvertPoint(points[0]));
        for (int i = 1; i < n; i++)
        {
            path.lineTo(wxQtConvertPoint(points[i]));
        }

        m_qtPainter.translate(xoffset, yoffset);
        m_qtPainter.drawPath(path);

        // Reset transform
        ComputeScaleAndOrigin();
    }
}

void wxQtDCImpl::DoDrawPolygon(int n, wxPoint points[],
                       wxCoord xoffset, wxCoord yoffset,
                       wxPolygonFillMode fillStyle )
{
    QPolygon qtPoints;
    for (int i = 0; i < n; i++) {
        qtPoints << wxQtConvertPoint(points[i]);
    }

    Qt::FillRule fill = (fillStyle == wxWINDING_RULE) ? Qt::WindingFill : Qt::OddEvenFill;
    
    m_qtPainter.translate(xoffset, yoffset);
    m_qtPainter.drawPolygon(qtPoints, fill);
    // Reset transform
    ComputeScaleAndOrigin();
}

void wxQtDCImpl::ComputeScaleAndOrigin()
{
    QTransform t;

    // First apply device origin
    t.translate( m_deviceOriginX + m_deviceLocalOriginX,
                 m_deviceOriginY + m_deviceLocalOriginY );

    // Second, scale
    m_scaleX = m_logicalScaleX * m_userScaleX;
    m_scaleY = m_logicalScaleY * m_userScaleY;
    t.scale( m_scaleX * m_signX, m_scaleY * m_signY );

    // Finally, logical origin
    t.translate( m_logicalOriginX, m_logicalOriginY );

    // Apply transform to QPainter, overwriting the previous one
    m_qtPainter.setWorldTransform(t, false);
}