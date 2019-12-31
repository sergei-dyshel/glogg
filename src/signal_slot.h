/*
 * Copyright (C) 2019 Sergei Dyshel
 * and other contributors
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

#pragma once

#include <type_traits>

#include <QtGlobal>

#define _S_METHOD_PTR(obj, method)                                             \
    &std::remove_pointer<decltype(obj)>::type::method

#define _S_METHOD_ARG(obj, method)                                             \
    MethodFirstArgument<decltype(_S_METHOD_PTR(obj, method))>::type

#define _S_OBJ_METHOD(obj, method) (obj), _S_METHOD_PTR(obj, method)

#define _S_OVERLOAD(obj, method, arg)                                          \
    (obj), qOverload<arg>(_S_METHOD_PTR(obj, method))

#define _S_IGNORE_ARG(obj, method, type) (obj), [=](type) { (obj)->method(); }

#define _S_LAMBDA_0ARG(obj, method)                                            \
    [=]() {                                                                    \
        if (obj)                                                               \
            (obj)->method();                                                   \
    }

#define _S_LAMBDA_1ARG(obj, method, type)                                      \
    [=](type arg) {                                                            \
        if (obj)                                                               \
            (obj)->method(arg);                                                \
    }

/*
 * Interface
 */

/**
 * Normal connect signal to slot
 */
#define CONNECT(sender, signal, receiver, slot)                                \
    QObject::connect(_S_OBJ_METHOD(sender, signal),                            \
                     _S_OBJ_METHOD(receiver, slot))

/**
 * Connect when signal has argument but slot doesn't
 */
#define CONNECT_1_TO_0_ARG(sender, signal, receiver, slot)                     \
    QObject::connect(                                                          \
        _S_OBJ_METHOD(sender, signal),                                         \
        _S_IGNORE_ARG(receiver, slot, _S_METHOD_ARG(sender, signal)))

/**
 * Choose overloaded signal to match slot argument
 */
#define CONNECT_OVLD_SIGNAL(sender, signal, receiver, slot)                    \
    QObject::connect(                                                          \
        _S_OVERLOAD(sender, signal, _S_METHOD_ARG(receiver, slot)),            \
        _S_OBJ_METHOD(receiver, slot))

/**
 * Choose overloaded slot to match signal argument
 */
#define CONNECT_OVLD_SLOT(sender, signal, receiver, slot)                      \
    QObject::connect(                                                          \
        _S_OBJ_METHOD(sender, signal),                                         \
        _S_OVERLOAD(receiver, slot, _S_METHOD_ARG(sender, signal)))

/**
 * Choose overloaded signal without arguments, slot shoud have no arguments too
 */
#define CONNECT_OVLD_0_ARG(sender, signal, receiver, slot)                     \
    QObject::connect(_S_OVERLOAD(sender, signal, ),                            \
                     _S_OVERLOAD(receiver, slot, ))

/**
 * Choose overloaded signal with given argument, slot shoud have no arguments
 */
#define CONNECT_OVLD_1_TO_0_ARG(sender, signal, arg, receiver, slot)           \
    QObject::connect(_S_OVERLOAD(sender, signal, arg),                         \
                     _S_OBJ_METHOD(receiver, slot))

template <typename> struct MethodFirstArgument;

template <typename C, typename A, typename... Args>
struct MethodFirstArgument<void (C::*)(A, Args...)> {
    using type = A;
};
