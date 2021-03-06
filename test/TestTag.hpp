// Copyright (c) 2013, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.

#include <nix/hydra/multiArray.hpp>
#include <nix.hpp>

#include <nix/hdf5/TagHDF5.hpp>
#include <nix/util/util.hpp>

#include <iostream>
#include <sstream>
#include <iterator>
#include <stdexcept>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <boost/math/constants/constants.hpp>


class TestTag: public CPPUNIT_NS::TestFixture {
private:

    nix::File file;
    nix::Block block;
    nix::DataArray positions, extents;
    nix::Tag tag, tag_other, tag_null;
    nix::Section section;
    time_t startup_time;
    std::vector<nix::DataArray> refs;

    CPPUNIT_TEST_SUITE(TestTag);

    CPPUNIT_TEST(testValidate);
    CPPUNIT_TEST(testId);
    CPPUNIT_TEST(testName);
    CPPUNIT_TEST(testType);
    CPPUNIT_TEST(testDefinition);
    CPPUNIT_TEST(testCreateRemove);
    CPPUNIT_TEST(testExtent);
    CPPUNIT_TEST(testPosition);
    CPPUNIT_TEST(testDataAccess);
    CPPUNIT_TEST(testMetadataAccess);
    CPPUNIT_TEST(testSourceAccess);
    CPPUNIT_TEST(testUnits);
    CPPUNIT_TEST(testReferences);
    CPPUNIT_TEST(testFeatures);
    CPPUNIT_TEST(testCreatedAt);
    CPPUNIT_TEST(testUpdatedAt);
    CPPUNIT_TEST(testOperators);
    CPPUNIT_TEST_SUITE_END ();

public:

    void setUp();
    void tearDown();

    void testValidate();

    void testId();
    void testName();
    void testType();
    void testDefinition();
    void testCreateRemove();
    void testExtent();
    void testPosition();
    void testDataAccess();
    void testMetadataAccess();
    void testSourceAccess();
    void testUnits();
    void testReferences();
    void testFeatures();
    void testOperators();
    void testCreatedAt();
    void testUpdatedAt();
};
