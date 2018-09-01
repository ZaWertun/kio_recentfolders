/***************************************************************************
 *   Copyright (C) 2016 by Arnav Dhamija <arnav.dhamija@gmail.com>         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef HELLO_H
#define HELLO_H

#include <kfileitem.h>
#include <kio/slavebase.h>

class RecentFolders : public KIO::SlaveBase
{
public:
    RecentFolders(const QByteArray &pool, const QByteArray &app);
    ~RecentFolders() override;

    void listDir(const QUrl&) override;
    void mimetype(const QUrl&) override;
    void stat(const QUrl&) override;

private:
    QString toPretty(const QString& path);
};

#endif
