# coding: utf-8
# Copyright (c) 2016, 2020, Oracle and/or its affiliates.  All rights reserved.
# This software is dual-licensed to you under the Universal Permissive License (UPL) 1.0 as shown at https://oss.oracle.com/licenses/upl or Apache License 2.0 as shown at http://www.apache.org/licenses/LICENSE-2.0. You may choose either license.

from .flow_port_link import FlowPortLink
from oci.util import formatted_flat_dict, NONE_SENTINEL, value_allowed_none_or_none_sentinel  # noqa: F401
from oci.decorators import init_model_state_from_kwargs


@init_model_state_from_kwargs
class OutputLink(FlowPortLink):
    """
    The information about output links.
    """

    def __init__(self, **kwargs):
        """
        Initializes a new OutputLink object with values from keyword arguments. The default value of the :py:attr:`~oci.data_integration.models.OutputLink.model_type` attribute
        of this class is ``OUTPUT_LINK`` and it should not be changed.
        The following keyword arguments are supported (corresponding to the getters/setters of this class):

        :param model_type:
            The value to assign to the model_type property of this OutputLink.
            Allowed values for this property are: "CONDITIONAL_INPUT_LINK", "OUTPUT_LINK", "INPUT_LINK"
        :type model_type: str

        :param key:
            The value to assign to the key property of this OutputLink.
        :type key: str

        :param model_version:
            The value to assign to the model_version property of this OutputLink.
        :type model_version: str

        :param parent_ref:
            The value to assign to the parent_ref property of this OutputLink.
        :type parent_ref: ParentReference

        :param object_status:
            The value to assign to the object_status property of this OutputLink.
        :type object_status: int

        :param description:
            The value to assign to the description property of this OutputLink.
        :type description: str

        :param port:
            The value to assign to the port property of this OutputLink.
        :type port: str

        :param to_links:
            The value to assign to the to_links property of this OutputLink.
        :type to_links: list[str]

        """
        self.swagger_types = {
            'model_type': 'str',
            'key': 'str',
            'model_version': 'str',
            'parent_ref': 'ParentReference',
            'object_status': 'int',
            'description': 'str',
            'port': 'str',
            'to_links': 'list[str]'
        }

        self.attribute_map = {
            'model_type': 'modelType',
            'key': 'key',
            'model_version': 'modelVersion',
            'parent_ref': 'parentRef',
            'object_status': 'objectStatus',
            'description': 'description',
            'port': 'port',
            'to_links': 'toLinks'
        }

        self._model_type = None
        self._key = None
        self._model_version = None
        self._parent_ref = None
        self._object_status = None
        self._description = None
        self._port = None
        self._to_links = None
        self._model_type = 'OUTPUT_LINK'

    @property
    def to_links(self):
        """
        Gets the to_links of this OutputLink.
        The links from this output link to connect to other links in flow.


        :return: The to_links of this OutputLink.
        :rtype: list[str]
        """
        return self._to_links

    @to_links.setter
    def to_links(self, to_links):
        """
        Sets the to_links of this OutputLink.
        The links from this output link to connect to other links in flow.


        :param to_links: The to_links of this OutputLink.
        :type: list[str]
        """
        self._to_links = to_links

    def __repr__(self):
        return formatted_flat_dict(self)

    def __eq__(self, other):
        if other is None:
            return False

        return self.__dict__ == other.__dict__

    def __ne__(self, other):
        return not self == other
