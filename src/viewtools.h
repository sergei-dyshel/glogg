/*
 * Copyright (C) 2015 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIEWTOOLS_H
#define VIEWTOOLS_H

#include <chrono>

#include <QObject>

// This class is a controller for an elastic hook manipulated with
// the mouse wheel or touchpad.
// It is used for the "follow" line at the end of the file.
class ElasticHook : public QObject {
  Q_OBJECT

  public:
    ElasticHook( int hook_threshold ) : hook_threshold_( hook_threshold ) {}

    // Instruct the elastic to move by the passed pixels
    // (a positive value increase the elastic tension)
    void move( int value );

    int length() const { return position_; }
    bool isHooked() const { return hooked_; }

  protected:
    void timerEvent( QTimerEvent *event );

  signals:
    // Sent when the length has changed
    void lengthChanged();
    // Sent when the hooked status has changed
    void hooked( bool is_hooked );

  private:
    void decreasePosition();

    static constexpr int TIMER_PERIOD_MS = 10;
    static constexpr int DECREASE_RATE = 4;
    const int hook_threshold_;
    bool hooked_  = false;
    int position_ = 0;
    int timer_id_ = 0;
    std::chrono::time_point<std::chrono::steady_clock> last_update_;
};

#endif