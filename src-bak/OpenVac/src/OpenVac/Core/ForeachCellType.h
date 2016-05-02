// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_FOREACHCELLTYPE_H
#define OPENVAC_FOREACHCELLTYPE_H

/// \file OpenVac/Core/ForeachCellType.h
/// \brief Defines macros to loop over each cell type


/***************************** Without arguments *****************************/

#define OPENVAC_FOREACH_BASE_CELL_TYPE(MACRO_NAME) \
    MACRO_NAME(Cell)

#define OPENVAC_FOREACH_MID_CELL_TYPE(MACRO_NAME) \
    MACRO_NAME(KeyCell) \
    MACRO_NAME(VertexCell) \
    MACRO_NAME(EdgeCell)

#define OPENVAC_FOREACH_FINAL_CELL_TYPE(MACRO_NAME) \
    MACRO_NAME(KeyVertex) \
    MACRO_NAME(KeyEdge)

#define OPENVAC_FOREACH_CELL_TYPE(MACRO_NAME) \
    OPENVAC_FOREACH_BASE_CELL_TYPE  (MACRO_NAME) \
    OPENVAC_FOREACH_MID_CELL_TYPE   (MACRO_NAME) \
    OPENVAC_FOREACH_FINAL_CELL_TYPE (MACRO_NAME)

#define OPENVAC_FOREACH_DERIVED_CELL_TYPE(MACRO_NAME) \
    OPENVAC_FOREACH_MID_CELL_TYPE   (MACRO_NAME) \
    OPENVAC_FOREACH_FINAL_CELL_TYPE (MACRO_NAME)

#define OPENVAC_FOREACH_CELL_DATA_TYPE(MACRO_NAME) \
    OPENVAC_FOREACH_BASE_CELL_TYPE  (MACRO_NAME) \
    OPENVAC_FOREACH_FINAL_CELL_TYPE (MACRO_NAME)


/*************************** With variadic arguments *************************/

#define OPENVAC_FOREACH_BASE_CELL_TYPE_ARGS(MACRO_NAME, ...) \
    MACRO_NAME (Cell, __VA_ARGS__)

#define OPENVAC_FOREACH_MID_CELL_TYPE_ARGS(MACRO_NAME, ...) \
    MACRO_NAME (KeyCell,    __VA_ARGS__) \
    MACRO_NAME (VertexCell, __VA_ARGS__) \
    MACRO_NAME (EdgeCell,   __VA_ARGS__)

#define OPENVAC_FOREACH_FINAL_CELL_TYPE_ARGS(MACRO_NAME, ...) \
    MACRO_NAME (KeyVertex, __VA_ARGS__) \
    MACRO_NAME (KeyEdge,   __VA_ARGS__)

#define OPENVAC_FOREACH_CELL_TYPE_ARGS(MACRO_NAME, ...) \
    OPENVAC_FOREACH_BASE_CELL_TYPE_ARGS  (MACRO_NAME, __VA_ARGS__) \
    OPENVAC_FOREACH_MID_CELL_TYPE_ARGS   (MACRO_NAME, __VA_ARGS__) \
    OPENVAC_FOREACH_FINAL_CELL_TYPE_ARGS (MACRO_NAME, __VA_ARGS__)

#define OPENVAC_FOREACH_DERIVED_CELL_TYPE_ARGS(MACRO_NAME, ...) \
    OPENVAC_FOREACH_MID_CELL_TYPE_ARGS   (MACRO_NAME, __VA_ARGS__) \
    OPENVAC_FOREACH_FINAL_CELL_TYPE_ARGS (MACRO_NAME, __VA_ARGS__)

#define OPENVAC_FOREACH_CELL_DATA_TYPE_ARGS(MACRO_NAME, ...) \
    OPENVAC_FOREACH_BASE_CELL_TYPE_ARGS  (MACRO_NAME, __VA_ARGS__) \
    OPENVAC_FOREACH_FINAL_CELL_TYPE_ARGS (MACRO_NAME, __VA_ARGS__)


#endif // OPENVAC_FOREACHCELLTYPE_H