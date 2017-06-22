/*
 * Copyright (C) 2013, 2014 Nicolas Bonnefon and other contributors
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

#ifndef CFG_H
#define CFG_H

#include <assert.h>

#include <QJsonValue>
#include <QJsonObject>

#include "map.h"

#define CFG_SCALAR_MEMBER( type, name, setter, default )                       \
    type name() const { return name##_; }                                      \
    void setter( const type& new_value )                                       \
    {                                                                          \
        name##_ = new_value;                                                   \
        childUpdated();                                                        \
    }                                                                          \
    type name##_ = default;

#define CFG_INT( ... ) CFG_SCALAR_MEMBER( qint64, __VA_ARGS__ )

#define CFG_MEMBER_FROM_JSON( name )                                           \
    if ( object.contains( #name ) )                                            \
        name##_ = object.value( #name );

#define CFG_DEFAULT_FROM_JSON( ... )                                           \
    void fromJson( const QJsonValue& value ) override                          \
    {                                                                          \
        if ( !value.isObject() )                                               \
            return;                                                            \
        QJsonObject object = value.toObject();                                 \
        MAP( CFG_MEMBER_FROM_JSON, __VA_ARGS__ )                               \
    }

#define CFG_MEMBER_TO_JSON( name ) object[ #name ] = name##_;

#define CFG_DEFAULT_TO_JSON( ... )                                             \
    QJsonValue toJson() const override                                         \
    {                                                                          \
        QJsonObject object;                                                    \
        MAP( CFG_MEMBER_TO_JSON, __VA_ARGS__ )                               \
        return object;                                                         \
    }

#define CFG_DEFAULT_FROM_TO_JSON( ... )                                        \
    CFG_DEFAULT_FROM_JSON(__VA_ARGS__ )                                      \
    CFG_DEFAULT_TO_JSON(__VA_ARGS__ )

namespace cfg {

class JsonSerializable {
    virtual void fromJson( const QJsonValue& ) = 0;
    virtual QJsonValue toJson() const = 0;

  protected:
    void setParent( JsonSerializable* new_parent )
    {
        assert( parent_ == nullptr );
        parent_ = new_parent;
    }
    virtual void childUpdated()
    {
        assert( parent_ != nullptr );
        parent_->childUpdated();
    }

  private:
    JsonSerializable* parent_ = nullptr;
};


} // namespace cfg

#endif /* CFG_H */
