# coding: utf-8
# Copyright (c) 2016, 2020, Oracle and/or its affiliates.  All rights reserved.
# This software is dual-licensed to you under the Universal Permissive License (UPL) 1.0 as shown at https://oss.oracle.com/licenses/upl or Apache License 2.0 as shown at http://www.apache.org/licenses/LICENSE-2.0. You may choose either license.

from __future__ import absolute_import

from .free_text_search_details import FreeTextSearchDetails
from .queryable_field_description import QueryableFieldDescription
from .resource_summary import ResourceSummary
from .resource_summary_collection import ResourceSummaryCollection
from .resource_type import ResourceType
from .search_context import SearchContext
from .search_details import SearchDetails
from .structured_search_details import StructuredSearchDetails

# Maps type names to classes for resource_search services.
resource_search_type_mapping = {
    "FreeTextSearchDetails": FreeTextSearchDetails,
    "QueryableFieldDescription": QueryableFieldDescription,
    "ResourceSummary": ResourceSummary,
    "ResourceSummaryCollection": ResourceSummaryCollection,
    "ResourceType": ResourceType,
    "SearchContext": SearchContext,
    "SearchDetails": SearchDetails,
    "StructuredSearchDetails": StructuredSearchDetails
}
