# coding: utf-8
# Copyright (c) 2016, 2020, Oracle and/or its affiliates.  All rights reserved.
# This software is dual-licensed to you under the Universal Permissive License (UPL) 1.0 as shown at https://oss.oracle.com/licenses/upl or Apache License 2.0 as shown at http://www.apache.org/licenses/LICENSE-2.0. You may choose either license.


from oci.util import formatted_flat_dict, NONE_SENTINEL, value_allowed_none_or_none_sentinel  # noqa: F401
from oci.decorators import init_model_state_from_kwargs


@init_model_state_from_kwargs
class ComputeImageCapabilitySchemaSummary(object):
    """
    Summary information for a compute image capability schema
    """

    def __init__(self, **kwargs):
        """
        Initializes a new ComputeImageCapabilitySchemaSummary object with values from keyword arguments.
        The following keyword arguments are supported (corresponding to the getters/setters of this class):

        :param id:
            The value to assign to the id property of this ComputeImageCapabilitySchemaSummary.
        :type id: str

        :param compartment_id:
            The value to assign to the compartment_id property of this ComputeImageCapabilitySchemaSummary.
        :type compartment_id: str

        :param compute_global_image_capability_schema_version_name:
            The value to assign to the compute_global_image_capability_schema_version_name property of this ComputeImageCapabilitySchemaSummary.
        :type compute_global_image_capability_schema_version_name: str

        :param image_id:
            The value to assign to the image_id property of this ComputeImageCapabilitySchemaSummary.
        :type image_id: str

        :param display_name:
            The value to assign to the display_name property of this ComputeImageCapabilitySchemaSummary.
        :type display_name: str

        :param time_created:
            The value to assign to the time_created property of this ComputeImageCapabilitySchemaSummary.
        :type time_created: datetime

        :param defined_tags:
            The value to assign to the defined_tags property of this ComputeImageCapabilitySchemaSummary.
        :type defined_tags: dict(str, dict(str, object))

        :param freeform_tags:
            The value to assign to the freeform_tags property of this ComputeImageCapabilitySchemaSummary.
        :type freeform_tags: dict(str, str)

        """
        self.swagger_types = {
            'id': 'str',
            'compartment_id': 'str',
            'compute_global_image_capability_schema_version_name': 'str',
            'image_id': 'str',
            'display_name': 'str',
            'time_created': 'datetime',
            'defined_tags': 'dict(str, dict(str, object))',
            'freeform_tags': 'dict(str, str)'
        }

        self.attribute_map = {
            'id': 'id',
            'compartment_id': 'compartmentId',
            'compute_global_image_capability_schema_version_name': 'computeGlobalImageCapabilitySchemaVersionName',
            'image_id': 'imageId',
            'display_name': 'displayName',
            'time_created': 'timeCreated',
            'defined_tags': 'definedTags',
            'freeform_tags': 'freeformTags'
        }

        self._id = None
        self._compartment_id = None
        self._compute_global_image_capability_schema_version_name = None
        self._image_id = None
        self._display_name = None
        self._time_created = None
        self._defined_tags = None
        self._freeform_tags = None

    @property
    def id(self):
        """
        **[Required]** Gets the id of this ComputeImageCapabilitySchemaSummary.
        The compute image capability schema `OCID`__.

        __ https://docs.cloud.oracle.com/Content/General/Concepts/identifiers.htm


        :return: The id of this ComputeImageCapabilitySchemaSummary.
        :rtype: str
        """
        return self._id

    @id.setter
    def id(self, id):
        """
        Sets the id of this ComputeImageCapabilitySchemaSummary.
        The compute image capability schema `OCID`__.

        __ https://docs.cloud.oracle.com/Content/General/Concepts/identifiers.htm


        :param id: The id of this ComputeImageCapabilitySchemaSummary.
        :type: str
        """
        self._id = id

    @property
    def compartment_id(self):
        """
        Gets the compartment_id of this ComputeImageCapabilitySchemaSummary.
        The OCID of the compartment containing the compute global image capability schema


        :return: The compartment_id of this ComputeImageCapabilitySchemaSummary.
        :rtype: str
        """
        return self._compartment_id

    @compartment_id.setter
    def compartment_id(self, compartment_id):
        """
        Sets the compartment_id of this ComputeImageCapabilitySchemaSummary.
        The OCID of the compartment containing the compute global image capability schema


        :param compartment_id: The compartment_id of this ComputeImageCapabilitySchemaSummary.
        :type: str
        """
        self._compartment_id = compartment_id

    @property
    def compute_global_image_capability_schema_version_name(self):
        """
        **[Required]** Gets the compute_global_image_capability_schema_version_name of this ComputeImageCapabilitySchemaSummary.
        The name of the compute global image capability schema version


        :return: The compute_global_image_capability_schema_version_name of this ComputeImageCapabilitySchemaSummary.
        :rtype: str
        """
        return self._compute_global_image_capability_schema_version_name

    @compute_global_image_capability_schema_version_name.setter
    def compute_global_image_capability_schema_version_name(self, compute_global_image_capability_schema_version_name):
        """
        Sets the compute_global_image_capability_schema_version_name of this ComputeImageCapabilitySchemaSummary.
        The name of the compute global image capability schema version


        :param compute_global_image_capability_schema_version_name: The compute_global_image_capability_schema_version_name of this ComputeImageCapabilitySchemaSummary.
        :type: str
        """
        self._compute_global_image_capability_schema_version_name = compute_global_image_capability_schema_version_name

    @property
    def image_id(self):
        """
        **[Required]** Gets the image_id of this ComputeImageCapabilitySchemaSummary.
        The OCID of the image associated with this compute image capability schema


        :return: The image_id of this ComputeImageCapabilitySchemaSummary.
        :rtype: str
        """
        return self._image_id

    @image_id.setter
    def image_id(self, image_id):
        """
        Sets the image_id of this ComputeImageCapabilitySchemaSummary.
        The OCID of the image associated with this compute image capability schema


        :param image_id: The image_id of this ComputeImageCapabilitySchemaSummary.
        :type: str
        """
        self._image_id = image_id

    @property
    def display_name(self):
        """
        **[Required]** Gets the display_name of this ComputeImageCapabilitySchemaSummary.
        A user-friendly name for the compute image capability schema.


        :return: The display_name of this ComputeImageCapabilitySchemaSummary.
        :rtype: str
        """
        return self._display_name

    @display_name.setter
    def display_name(self, display_name):
        """
        Sets the display_name of this ComputeImageCapabilitySchemaSummary.
        A user-friendly name for the compute image capability schema.


        :param display_name: The display_name of this ComputeImageCapabilitySchemaSummary.
        :type: str
        """
        self._display_name = display_name

    @property
    def time_created(self):
        """
        **[Required]** Gets the time_created of this ComputeImageCapabilitySchemaSummary.
        The date and time the compute image capability schema was created, in the format defined by `RFC3339`__.

        Example: `2016-08-25T21:10:29.600Z`

        __ https://tools.ietf.org/html/rfc3339


        :return: The time_created of this ComputeImageCapabilitySchemaSummary.
        :rtype: datetime
        """
        return self._time_created

    @time_created.setter
    def time_created(self, time_created):
        """
        Sets the time_created of this ComputeImageCapabilitySchemaSummary.
        The date and time the compute image capability schema was created, in the format defined by `RFC3339`__.

        Example: `2016-08-25T21:10:29.600Z`

        __ https://tools.ietf.org/html/rfc3339


        :param time_created: The time_created of this ComputeImageCapabilitySchemaSummary.
        :type: datetime
        """
        self._time_created = time_created

    @property
    def defined_tags(self):
        """
        Gets the defined_tags of this ComputeImageCapabilitySchemaSummary.
        Defined tags for this resource. Each key is predefined and scoped to a
        namespace. For more information, see `Resource Tags`__.

        Example: `{\"Operations\": {\"CostCenter\": \"42\"}}`

        __ https://docs.cloud.oracle.com/Content/General/Concepts/resourcetags.htm


        :return: The defined_tags of this ComputeImageCapabilitySchemaSummary.
        :rtype: dict(str, dict(str, object))
        """
        return self._defined_tags

    @defined_tags.setter
    def defined_tags(self, defined_tags):
        """
        Sets the defined_tags of this ComputeImageCapabilitySchemaSummary.
        Defined tags for this resource. Each key is predefined and scoped to a
        namespace. For more information, see `Resource Tags`__.

        Example: `{\"Operations\": {\"CostCenter\": \"42\"}}`

        __ https://docs.cloud.oracle.com/Content/General/Concepts/resourcetags.htm


        :param defined_tags: The defined_tags of this ComputeImageCapabilitySchemaSummary.
        :type: dict(str, dict(str, object))
        """
        self._defined_tags = defined_tags

    @property
    def freeform_tags(self):
        """
        Gets the freeform_tags of this ComputeImageCapabilitySchemaSummary.
        Free-form tags for this resource. Each tag is a simple key-value pair with no
        predefined name, type, or namespace. For more information, see `Resource Tags`__.

        Example: `{\"Department\": \"Finance\"}`

        __ https://docs.cloud.oracle.com/Content/General/Concepts/resourcetags.htm


        :return: The freeform_tags of this ComputeImageCapabilitySchemaSummary.
        :rtype: dict(str, str)
        """
        return self._freeform_tags

    @freeform_tags.setter
    def freeform_tags(self, freeform_tags):
        """
        Sets the freeform_tags of this ComputeImageCapabilitySchemaSummary.
        Free-form tags for this resource. Each tag is a simple key-value pair with no
        predefined name, type, or namespace. For more information, see `Resource Tags`__.

        Example: `{\"Department\": \"Finance\"}`

        __ https://docs.cloud.oracle.com/Content/General/Concepts/resourcetags.htm


        :param freeform_tags: The freeform_tags of this ComputeImageCapabilitySchemaSummary.
        :type: dict(str, str)
        """
        self._freeform_tags = freeform_tags

    def __repr__(self):
        return formatted_flat_dict(self)

    def __eq__(self, other):
        if other is None:
            return False

        return self.__dict__ == other.__dict__

    def __ne__(self, other):
        return not self == other
