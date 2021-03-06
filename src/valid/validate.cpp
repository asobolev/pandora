// Copyright (c) 2014, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.

#include <nix/valid/validator.hpp>
#include <nix/valid/checks.hpp>
#include <nix/valid/conditions.hpp>
#include <nix/valid/result.hpp>

#include <nix.hpp>

namespace nix {
namespace valid {

// ---------------------------------------------------------------------
// Templated, hidden validation utils that are only here in the cpp,
// not in the header (hides them from user)
// ---------------------------------------------------------------------

/**
  * @brief base entity validator
  * 
  * Function taking a base entity and returning {@link Result} object
  *
  * @param entity base entity
  *
  * @returns The validation results as {@link Result} object
  */
template<typename T>
Result validate_entity(const base::Entity<T> &entity) {
    return validator({
        must(entity, &base::Entity<T>::id, notEmpty(), "id is not set!"),
        must(entity, &base::Entity<T>::createdAt, notFalse(), "date is not set!")
    });
}

/**
  * @brief base named entity validator
  * 
  * Function taking a base named entity and returning {@link Result}
  * object
  *
  * @param named_entity base named entity
  *
  * @returns The validation results as {@link Result} object
  */
template<typename T>
Result validate_named_entity(const base::NamedEntity<T> &named_entity) {
    Result result_base = validate_entity<T>(named_entity);
    Result result = validator({
        must(named_entity, &base::NamedEntity<T>::name, notEmpty(), "no name set!"),
        must(named_entity, &base::NamedEntity<T>::type, notEmpty(), "no type set!")
    });

    return result.concat(result_base);
}

/**
  * @brief base entity with metadata validator
  * 
  * Function taking a base entity with metadata and returning
  * {@link Result} object
  *
  * @param entity_with_metadata base entity with metadata
  *
  * @returns The validation results as {@link Result} object
  */
template<typename T>
Result validate_entity_with_metadata(const base::EntityWithMetadata<T> &entity_with_metadata) {
    return validate_named_entity<T>(entity_with_metadata);
}

/**
  * @brief base entity with sources validator
  * 
  * Function taking a base entity with sources and returning
  * {@link Result} object
  *
  * @param entity_with_sources base entity with sources
  *
  * @returns The validation results as {@link Result} object
  */
template<typename T>
Result validate_entity_with_sources(const base::EntityWithSources<T> &entity_with_sources) {
    return validate_entity_with_metadata<T>(entity_with_sources);
}

// ---------------------------------------------------------------------
// Regular validaton utils split in header & cpp part
// ---------------------------------------------------------------------

Result validate(const Block &block) {
    return validate_entity_with_metadata(block);
}

Result validate(const DataArray &data_array) {
    Result result_base = validate_entity_with_sources(data_array);
    Result result = validator({
        must(data_array, &DataArray::dataType, notEqual<DataType>(DataType::Nothing), "data type is not set!"),
        must(data_array, &DataArray::dimensionCount, isEqual<size_t>(data_array.dataExtent().size()), "data dimensionality does not match number of defined dimensions!", {
            could(data_array, &DataArray::dimensions, notEmpty(), {
                must(data_array, &DataArray::dimensions, dimTicksMatchData(data_array), "in some of the Range dimensions the number of ticks differs from the number of data entries along the corresponding data dimension!"),
                must(data_array, &DataArray::dimensions, dimLabelsMatchData(data_array), "in some of the Set dimensions the number of labels differs from the number of data entries along the corresponding data dimension!") }) }),
        could(data_array, &DataArray::unit, notFalse(), {
            must(data_array, &DataArray::unit, isValidUnit(), "Unit is not SI or composite of SI units.") }),
        could(data_array, &DataArray::polynomCoefficients, notEmpty(), {
            should(data_array, &DataArray::expansionOrigin, notFalse(), "polynomial coefficients for calibration are set, but expansion origin is missing!") }),
        could(data_array, &DataArray::expansionOrigin, notFalse(), {
            should(data_array, &DataArray::polynomCoefficients, notEmpty(), "expansion origin for calibration is set, but polynomial coefficients are missing!") })
    });

    return result.concat(result_base);
}

Result validate(const Tag &tag) {
    Result result_base = validate_entity_with_sources(tag);
    Result result = validator({
        must(tag, &Tag::position, notEmpty(), "position is not set!"),
        could(tag, &Tag::references, notEmpty(), {
            must(tag, &Tag::position, positionsMatchRefs(tag.references()),
                "number of entries in position does not match number of dimensions in all referenced DataArrays!"),
            could(tag, &Tag::extent, notEmpty(), {
                must(tag, &Tag::position, extentsMatchPositions(tag.extent()), "Number of entries in position and extent do not match!"),
                must(tag, &Tag::extent, extentsMatchRefs(tag.references()),
                    "number of entries in extent does not match number of dimensions in all referenced DataArrays!") })
        }),
        // check units for validity
        could(tag, &Tag::units, notEmpty(), {
            must(tag, &Tag::units, isValidUnit(), "Unit is invalid: not an atomic SI. Note: So far composite units are not supported!"),
            must(tag, &Tag::references, tagRefsHaveUnits(tag.units()), "Some of the referenced DataArrays' dimensions don't have units where the tag has. Make sure that all references have the same number of dimensions as the tag has units and that each dimension has a unit set."),
                must(tag, &Tag::references, tagUnitsMatchRefsUnits(tag.units()), "Some of the referenced DataArrays' dimensions have units that are not convertible to the units set in tag. Note: So far composite SI units are not supported!")}),
    });

    return result.concat(result_base);
}

Result validate(const Property &property) {
    Result result_base = validate_entity(property);
    Result result = validator({
        must(property, &Property::name, notEmpty(), "name is not set!"),
        could(property, &Property::valueCount, notFalse(), {
            should(property, &Property::unit, notFalse(), "values are set, but unit is missing!") }),
        could(property, &Property::unit, notFalse(), {
            must(property, &Property::unit, isValidUnit(), "Unit is not SI or composite of SI units.") })
        // TODO: dataType to be tested too?
    });

    return result.concat(result_base);
}

Result validate(const MultiTag &multi_tag) {
    Result result_base = validate_entity_with_sources(multi_tag);
    Result result = validator({
        must(multi_tag, &MultiTag::positions, notFalse(), "positions are not set!"),
        // since extents & positions DataArray stores a vector of position / extent vectors it has to be 2-dim
        could(multi_tag, &MultiTag::positions, notFalse(), {
            must(multi_tag, &MultiTag::positions, dimEquals(2), "dimensionality of positions DataArray must be two!") }),
        could(multi_tag, &MultiTag::extents, notFalse(), {
            must(multi_tag, &MultiTag::extents, dimEquals(2), "dimensionality of extents DataArray must be two!") }),
        // check units for validity
        could(multi_tag, &MultiTag::units, notEmpty(), {
            must(multi_tag, &MultiTag::units, isValidUnit(), "Some of the units in tag are invalid: not an atomic SI. Note: So far composite SI units are not supported!"),
            must(multi_tag, &MultiTag::references, tagUnitsMatchRefsUnits(multi_tag.units()), "Some of the referenced DataArrays' dimensions have units that are not convertible to the units set in tag. Note: So far composite SI units are not supported!")}),
        // check positions & extents
        could(multi_tag, &MultiTag::extents, notFalse(), {
            must(multi_tag, &MultiTag::positions, extentsMatchPositions(multi_tag.extents()), "Number of entries in positions and extents do not match!") }),
        could(multi_tag, &MultiTag::references, notEmpty(), {
            could(multi_tag, &MultiTag::extents, notFalse(), {
                must(multi_tag, &MultiTag::extents, extentsMatchRefs(multi_tag.references()), "number of entries (in 2nd dim) in extents does not match number of dimensions in all referenced DataArrays!") }),
            must(multi_tag, &MultiTag::positions, positionsMatchRefs(multi_tag.references()), "number of entries (in 2nd dim) in positions does not match number of dimensions in all referenced DataArrays!") })
    });

    return result.concat(result_base);
}

Result validate(const Dimension &dim) {
    return validator({
        must(dim, &Dimension::index, notSmaller(1), "index is not set to valid value (> 0)!")
    });
}

Result validate(const RangeDimension &range_dim) {
    return validator({
        must(range_dim, &RangeDimension::index, notSmaller(1), "index is not set to valid value (size_t > 0)!"),
        must(range_dim, &RangeDimension::ticks, notEmpty(), "ticks are not set!"),
        must(range_dim, &RangeDimension::dimensionType, isEqual<DimensionType>(DimensionType::Range), "dimension type is not correct!"),
        could(range_dim, &RangeDimension::unit, notFalse(), {
            must(range_dim, &RangeDimension::unit, isAtomicUnit(), "Unit is set but not an atomic SI. Note: So far composite units are not supported!") }),
        must(range_dim, &RangeDimension::ticks, isSorted(), "Ticks are not sorted!")
    });
}

Result validate(const SampledDimension &sampled_dim) {
    return validator({
        must(sampled_dim, &SampledDimension::index, notSmaller(1), "index is not set to valid value (size_t > 0)!"),
        must(sampled_dim, &SampledDimension::samplingInterval, isGreater(0), "samplingInterval is not set to valid value (> 0)!"),
        must(sampled_dim, &SampledDimension::dimensionType, isEqual<DimensionType>(DimensionType::Sample), "dimension type is not correct!"),
        could(sampled_dim, &SampledDimension::offset, notFalse(), {
            should(sampled_dim, &SampledDimension::unit, isAtomicUnit(), "offset is set, but no valid unit set!") }),
        could(sampled_dim, &SampledDimension::unit, notFalse(), {
            must(sampled_dim, &SampledDimension::unit, isAtomicUnit(), "Unit is set but not an atomic SI. Note: So far composite units are not supported!") })
    });
}

Result validate(const SetDimension &set_dim) {
    return validator({
        must(set_dim, &SetDimension::index, notSmaller(1), "index is not set to valid value (size_t > 0)!"),
        must(set_dim, &SetDimension::dimensionType, isEqual<DimensionType>(DimensionType::Set), "dimension type is not correct!")
    });
}

Result validate(const Feature &feature) {
    Result result_base = validate_entity(feature);
    Result result = validator({
        must(feature, &Feature::data, notFalse(), "data is not set!"),
        must(feature, &Feature::linkType, notSmaller(0), "linkType is not set!")
    });

    return result.concat(result_base);
}

Result validate(const Section &section) {
    return validate_named_entity(section);
}

Result validate(const Source &source) {
    return validate_entity_with_metadata(source);
}

Result validate(const File &file) {
    return validator({
        could(file, &File::isOpen, notFalse(), {
            must(file, &File::createdAt, notFalse(), "date is not set!"),
            should(file, &File::version, notEmpty(), "version is not set!"),
            should(file, &File::format, notEmpty(), "format is not set!"),
            should(file, &File::location, notEmpty(), "location is not set!") })
    });
}

} // namespace valid
} // namespace nix
