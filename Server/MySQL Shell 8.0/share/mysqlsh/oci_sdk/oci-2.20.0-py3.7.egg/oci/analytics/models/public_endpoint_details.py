# coding: utf-8
# Copyright (c) 2016, 2020, Oracle and/or its affiliates.  All rights reserved.
# This software is dual-licensed to you under the Universal Permissive License (UPL) 1.0 as shown at https://oss.oracle.com/licenses/upl or Apache License 2.0 as shown at http://www.apache.org/licenses/LICENSE-2.0. You may choose either license.

from .network_endpoint_details import NetworkEndpointDetails
from oci.util import formatted_flat_dict, NONE_SENTINEL, value_allowed_none_or_none_sentinel  # noqa: F401
from oci.decorators import init_model_state_from_kwargs


@init_model_state_from_kwargs
class PublicEndpointDetails(NetworkEndpointDetails):
    """
    Public endpoint configuration details.
    """

    def __init__(self, **kwargs):
        """
        Initializes a new PublicEndpointDetails object with values from keyword arguments. The default value of the :py:attr:`~oci.analytics.models.PublicEndpointDetails.network_endpoint_type` attribute
        of this class is ``PUBLIC`` and it should not be changed.
        The following keyword arguments are supported (corresponding to the getters/setters of this class):

        :param network_endpoint_type:
            The value to assign to the network_endpoint_type property of this PublicEndpointDetails.
            Allowed values for this property are: "PUBLIC", "PRIVATE"
        :type network_endpoint_type: str

        :param whitelisted_ips:
            The value to assign to the whitelisted_ips property of this PublicEndpointDetails.
        :type whitelisted_ips: list[str]

        :param whitelisted_vcns:
            The value to assign to the whitelisted_vcns property of this PublicEndpointDetails.
        :type whitelisted_vcns: list[VirtualCloudNetwork]

        """
        self.swagger_types = {
            'network_endpoint_type': 'str',
            'whitelisted_ips': 'list[str]',
            'whitelisted_vcns': 'list[VirtualCloudNetwork]'
        }

        self.attribute_map = {
            'network_endpoint_type': 'networkEndpointType',
            'whitelisted_ips': 'whitelistedIps',
            'whitelisted_vcns': 'whitelistedVcns'
        }

        self._network_endpoint_type = None
        self._whitelisted_ips = None
        self._whitelisted_vcns = None
        self._network_endpoint_type = 'PUBLIC'

    @property
    def whitelisted_ips(self):
        """
        Gets the whitelisted_ips of this PublicEndpointDetails.
        Source IP addresses or IP address ranges igress rules.


        :return: The whitelisted_ips of this PublicEndpointDetails.
        :rtype: list[str]
        """
        return self._whitelisted_ips

    @whitelisted_ips.setter
    def whitelisted_ips(self, whitelisted_ips):
        """
        Sets the whitelisted_ips of this PublicEndpointDetails.
        Source IP addresses or IP address ranges igress rules.


        :param whitelisted_ips: The whitelisted_ips of this PublicEndpointDetails.
        :type: list[str]
        """
        self._whitelisted_ips = whitelisted_ips

    @property
    def whitelisted_vcns(self):
        """
        Gets the whitelisted_vcns of this PublicEndpointDetails.
        Virtual Cloud Networks allowed to access this network endpoint.


        :return: The whitelisted_vcns of this PublicEndpointDetails.
        :rtype: list[VirtualCloudNetwork]
        """
        return self._whitelisted_vcns

    @whitelisted_vcns.setter
    def whitelisted_vcns(self, whitelisted_vcns):
        """
        Sets the whitelisted_vcns of this PublicEndpointDetails.
        Virtual Cloud Networks allowed to access this network endpoint.


        :param whitelisted_vcns: The whitelisted_vcns of this PublicEndpointDetails.
        :type: list[VirtualCloudNetwork]
        """
        self._whitelisted_vcns = whitelisted_vcns

    def __repr__(self):
        return formatted_flat_dict(self)

    def __eq__(self, other):
        if other is None:
            return False

        return self.__dict__ == other.__dict__

    def __ne__(self, other):
        return not self == other
