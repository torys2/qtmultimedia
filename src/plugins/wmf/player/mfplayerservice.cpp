/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qmediacontent.h"

#include <QtCore/qdebug.h>

#include "mfplayercontrol.h"
#if defined(HAVE_WIDGETS) && !defined(Q_WS_SIMULATOR)
#include "evr9videowindowcontrol.h"
#endif
#include "mfvideorenderercontrol.h"
#include "mfaudioendpointcontrol.h"
#include "mfaudioprobecontrol.h"
#include "mfvideoprobecontrol.h"
#include "mfplayerservice.h"
#include "mfplayersession.h"
#include "mfmetadatacontrol.h"

MFPlayerService::MFPlayerService(QObject *parent)
    : QMediaService(parent)
    , m_session(0)
#ifndef Q_WS_SIMULATOR
    , m_videoWindowControl(0)
#endif
    , m_videoRendererControl(0)
{
    m_audioEndpointControl = new MFAudioEndpointControl(this);
    m_session = new MFPlayerSession(this);
    m_player = new MFPlayerControl(m_session);
    m_metaDataControl = new MFMetaDataControl(this);
}

MFPlayerService::~MFPlayerService()
{
#ifndef Q_WS_SIMULATOR
    if (m_videoWindowControl)
        delete m_videoWindowControl;
#endif

    if (m_videoRendererControl)
        delete m_videoRendererControl;

    m_session->close();
    m_session->Release();
}

QMediaControl* MFPlayerService::requestControl(const char *name)
{
    if (qstrcmp(name, QMediaPlayerControl_iid) == 0) {
        return m_player;
    } else if (qstrcmp(name, QAudioOutputSelectorControl_iid) == 0) {
        return m_audioEndpointControl;
    } else if (qstrcmp(name, QMetaDataReaderControl_iid) == 0) {
        return m_metaDataControl;
    } else if (qstrcmp(name, QVideoRendererControl_iid) == 0) {
#if defined(HAVE_WIDGETS) && !defined(Q_WS_SIMULATOR)
        if (!m_videoRendererControl && !m_videoWindowControl) {
#else
        if (!m_videoRendererControl) {
#endif
            m_videoRendererControl = new MFVideoRendererControl;
            return m_videoRendererControl;
        }
#if defined(HAVE_WIDGETS) && !defined(Q_WS_SIMULATOR)
    } else if (qstrcmp(name, QVideoWindowControl_iid) == 0) {
        if (!m_videoRendererControl && !m_videoWindowControl) {
            m_videoWindowControl = new Evr9VideoWindowControl;
            return m_videoWindowControl;
        }
#endif
    } else if (qstrcmp(name,QMediaAudioProbeControl_iid) == 0) {
        if (m_session) {
            MFAudioProbeControl *probe = new MFAudioProbeControl(this);
            m_session->addProbe(probe);
            return probe;
        }
        return 0;
    } else if (qstrcmp(name,QMediaVideoProbeControl_iid) == 0) {
        if (m_session) {
            MFVideoProbeControl *probe = new MFVideoProbeControl(this);
            m_session->addProbe(probe);
            return probe;
        }
        return 0;
    }

    return 0;
}

void MFPlayerService::releaseControl(QMediaControl *control)
{
    if (!control) {
        qWarning("QMediaService::releaseControl():"
                " Attempted release of null control");
    } else if (control == m_videoRendererControl) {
        m_videoRendererControl->setSurface(0);
        delete m_videoRendererControl;
        m_videoRendererControl = 0;
        return;
#if defined(HAVE_WIDGETS) && !defined(Q_WS_SIMULATOR)
    } else if (control == m_videoWindowControl) {
        delete m_videoWindowControl;
        m_videoWindowControl = 0;
        return;
#endif
    }

    MFAudioProbeControl* audioProbe = qobject_cast<MFAudioProbeControl*>(control);
    if (audioProbe) {
        if (m_session)
            m_session->removeProbe(audioProbe);
        delete audioProbe;
        return;
    }

    MFVideoProbeControl* videoProbe = qobject_cast<MFVideoProbeControl*>(control);
    if (videoProbe) {
        if (m_session)
            m_session->removeProbe(videoProbe);
        delete videoProbe;
        return;
    }
}

MFAudioEndpointControl* MFPlayerService::audioEndpointControl() const
{
    return m_audioEndpointControl;
}

MFVideoRendererControl* MFPlayerService::videoRendererControl() const
{
    return m_videoRendererControl;
}

#if defined(HAVE_WIDGETS) && !defined(Q_WS_SIMULATOR)
Evr9VideoWindowControl* MFPlayerService::videoWindowControl() const
{
    return m_videoWindowControl;
}
#endif

MFMetaDataControl* MFPlayerService::metaDataControl() const
{
    return m_metaDataControl;
}
