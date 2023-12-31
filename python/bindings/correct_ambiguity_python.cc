/*
 * Copyright 2023 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(correct_ambiguity.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(8c0b901b5a03051775528cbe8ff5b93c)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <satellites/correct_ambiguity.h>
// pydoc.h is automatically generated in the build directory
#include <correct_ambiguity_pydoc.h>

void bind_correct_ambiguity(py::module& m)
{

    using correct_ambiguity    = gr::satellites::correct_ambiguity;


    py::class_<correct_ambiguity, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<correct_ambiguity>>(m, "correct_ambiguity", D(correct_ambiguity))

        .def(py::init(&correct_ambiguity::make),
           D(correct_ambiguity,make)
        )
        



        ;




}








