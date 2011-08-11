
#include <QDebug>
#include <QtXml/QDomNode>
#include <QEvent>
#include <QDragEnterEvent>
#include <QUrl>
#include <QPainter>

#include "mixxx.h"
#include "trackinfoobject.h"
#include "wwaveformviewer.h"

#include "waveform/widget/waveformwidgetabstract.h"

WWaveformViewer::WWaveformViewer(const char *group, QWidget * parent, Qt::WFlags f) :
    QWidget(parent)
{
    m_pGroup = group;

    setAcceptDrops(true);

    installEventFilter(this);

    setAttribute(Qt::WA_OpaquePaintEvent,true);

    m_zoomZoneWidth = 20;
    m_waveformWidget = 0;
}

WWaveformViewer::~WWaveformViewer() {
}

void WWaveformViewer::setup(QDomNode node)
{
    if( m_waveformWidget)
        m_waveformWidget->setup(node);
}

void WWaveformViewer::resizeEvent(QResizeEvent* /*event*/)
{
    qDebug() << "WWaveformViewer::resizeEvent" << this << size();
    qDebug() << parent() << parentWidget() << parentWidget()->size();

    if( m_waveformWidget)
        m_waveformWidget->resize(width(),height());
}

bool WWaveformViewer::eventFilter(QObject *o, QEvent *e) {
    if(e->type() == QEvent::MouseButtonPress) {
        QMouseEvent *m = (QMouseEvent*)e;
        m_iMouseStart= -1;
        if(m->button() == Qt::LeftButton) {
            // The left button went down, so store the start position
            m_iMouseStart = m->x();
            emit(valueChangedLeftDown(64));
        }
    } else if(e->type() == QEvent::MouseMove) {
        // Only send signals for mouse moving if the left button is pressed
        if(m_iMouseStart != -1) {
            QMouseEvent *m = (QMouseEvent*)e;

            // start at the middle of 0-127, and emit values based on
            // how far the mouse has travelled horizontally
            double v = 64 + (double)(m->x()-m_iMouseStart)/10;
            // clamp to 0-127
            if(v<0)
                v = 0;
            else if(v > 127)
                v = 127;
            emit(valueChangedLeftDown(v));

        }
    } else if(e->type() == QEvent::MouseButtonRelease) {
        emit(valueChangedLeftDown(64));
    } else {
        return QObject::eventFilter(o,e);
    }
    return true;
}

void WWaveformViewer::wheelEvent(QWheelEvent *event)
{
    if( m_waveformWidget)
    {
        if( event->x() > width() - m_zoomZoneWidth)
        {
            if( event->delta() > 0)
                m_waveformWidget->zoomIn();
            else
                m_waveformWidget->zoomOut();
        }
        update();
    }
}

/** DRAG AND DROP **/

void WWaveformViewer::dragEnterEvent(QDragEnterEvent * event)
{
    // Accept the enter event if the thing is a filepath.
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void WWaveformViewer::dropEvent(QDropEvent * event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls(event->mimeData()->urls());
        QUrl url = urls.first();
        QString name = url.toLocalFile();
        //If the file is on a network share, try just converting the URL to a string...
        if (name == "")
            name = url.toString();

        event->accept();
        emit(trackDropped(name, m_pGroup));
    } else {
        event->ignore();
    }
}

void WWaveformViewer::onTrackLoaded( TrackPointer track)
{
    if( m_waveformWidget)
        m_waveformWidget->setTrack(track);
}

void WWaveformViewer::onTrackUnloaded( TrackPointer /*track*/)
{
    if( m_waveformWidget)
        m_waveformWidget->setTrack( TrackPointer(0));
}
