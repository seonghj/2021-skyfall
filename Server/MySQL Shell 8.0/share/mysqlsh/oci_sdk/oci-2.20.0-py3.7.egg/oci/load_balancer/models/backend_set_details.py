# coding: utf-8
# Copyright (c) 2016, 2020, Oracle and/or its affiliates.  All rights reserved.
# This software is dual-licensed to you under the Universal Permissive License (UPL) 1.0 as shown at https://oss.oracle.com/licenses/upl or Apache License 2.0 as shown at http://www.apache.org/licenses/LICENSE-2.0. You may choose either license.


from oci.util import formatted_flat_dict, NONE_SENTINEL, value_allowed_none_or_none_sentinel  # noqa: F401
from oci.decorators import init_model_state_from_kwargs


@init_model_state_from_kwargs
class BackendSetDetails(object):
    """
    The configuration details for a load balancer backend set.
    For more information on backend set configuration, see
    `Managing Backend Sets`__.

    **Note:** The `sessionPersistenceConfiguration` (application cookie stickiness) and `lbCookieSessionPersistenceConfiguration`
    (LB cookie stickiness) attributes are mutually exclusive. To avoid returning an error, configure only one of these two
    attributes per backend set.

    __ https://docs.cloud.oracle.com/Content/Balance/Tasks/managingbackendsets.htm
    """

    def __init__(self, **kwargs):
        """
        Initializes a new BackendSetDetails object with values from keyword arguments.
        The following keyword arguments are supported (corresponding to the getters/setters of this class):

        :param policy:
            The value to assign to the policy property of this BackendSetDetails.
        :type policy: str

        :param backends:
            The value to assign to the backends property of this BackendSetDetails.
        :type backends: list[BackendDetails]

        :param health_checker:
            The value to assign to the health_checker property of this BackendSetDetails.
        :type health_checker: HealthCheckerDetails

        :param ssl_configuration:
            The value to assign to the ssl_configuration property of this BackendSetDetails.
        :type ssl_configuration: SSLConfigurationDetails

        :param session_persistence_configuration:
            The value to assign to the session_persistence_configuration property of this BackendSetDetails.
        :type session_persistence_configuration: SessionPersistenceConfigurationDetails

        :param lb_cookie_session_persistence_configuration:
            The value to assign to the lb_cookie_session_persistence_configuration property of this BackendSetDetails.
        :type lb_cookie_session_persistence_configuration: LBCookieSessionPersistenceConfigurationDetails

        """
        self.swagger_types = {
            'policy': 'str',
            'backends': 'list[BackendDetails]',
            'health_checker': 'HealthCheckerDetails',
            'ssl_configuration': 'SSLConfigurationDetails',
            'session_persistence_configuration': 'SessionPersistenceConfigurationDetails',
            'lb_cookie_session_persistence_configuration': 'LBCookieSessionPersistenceConfigurationDetails'
        }

        self.attribute_map = {
            'policy': 'policy',
            'backends': 'backends',
            'health_checker': 'healthChecker',
            'ssl_configuration': 'sslConfiguration',
            'session_persistence_configuration': 'sessionPersistenceConfiguration',
            'lb_cookie_session_persistence_configuration': 'lbCookieSessionPersistenceConfiguration'
        }

        self._policy = None
        self._backends = None
        self._health_checker = None
        self._ssl_configuration = None
        self._session_persistence_configuration = None
        self._lb_cookie_session_persistence_configuration = None

    @property
    def policy(self):
        """
        **[Required]** Gets the policy of this BackendSetDetails.
        The load balancer policy for the backend set. To get a list of available policies, use the
        :func:`list_policies` operation.

        Example: `LEAST_CONNECTIONS`


        :return: The policy of this BackendSetDetails.
        :rtype: str
        """
        return self._policy

    @policy.setter
    def policy(self, policy):
        """
        Sets the policy of this BackendSetDetails.
        The load balancer policy for the backend set. To get a list of available policies, use the
        :func:`list_policies` operation.

        Example: `LEAST_CONNECTIONS`


        :param policy: The policy of this BackendSetDetails.
        :type: str
        """
        self._policy = policy

    @property
    def backends(self):
        """
        Gets the backends of this BackendSetDetails.

        :return: The backends of this BackendSetDetails.
        :rtype: list[BackendDetails]
        """
        return self._backends

    @backends.setter
    def backends(self, backends):
        """
        Sets the backends of this BackendSetDetails.

        :param backends: The backends of this BackendSetDetails.
        :type: list[BackendDetails]
        """
        self._backends = backends

    @property
    def health_checker(self):
        """
        **[Required]** Gets the health_checker of this BackendSetDetails.

        :return: The health_checker of this BackendSetDetails.
        :rtype: HealthCheckerDetails
        """
        return self._health_checker

    @health_checker.setter
    def health_checker(self, health_checker):
        """
        Sets the health_checker of this BackendSetDetails.

        :param health_checker: The health_checker of this BackendSetDetails.
        :type: HealthCheckerDetails
        """
        self._health_checker = health_checker

    @property
    def ssl_configuration(self):
        """
        Gets the ssl_configuration of this BackendSetDetails.

        :return: The ssl_configuration of this BackendSetDetails.
        :rtype: SSLConfigurationDetails
        """
        return self._ssl_configuration

    @ssl_configuration.setter
    def ssl_configuration(self, ssl_configuration):
        """
        Sets the ssl_configuration of this BackendSetDetails.

        :param ssl_configuration: The ssl_configuration of this BackendSetDetails.
        :type: SSLConfigurationDetails
        """
        self._ssl_configuration = ssl_configuration

    @property
    def session_persistence_configuration(self):
        """
        Gets the session_persistence_configuration of this BackendSetDetails.

        :return: The session_persistence_configuration of this BackendSetDetails.
        :rtype: SessionPersistenceConfigurationDetails
        """
        return self._session_persistence_configuration

    @session_persistence_configuration.setter
    def session_persistence_configuration(self, session_persistence_configuration):
        """
        Sets the session_persistence_configuration of this BackendSetDetails.

        :param session_persistence_configuration: The session_persistence_configuration of this BackendSetDetails.
        :type: SessionPersistenceConfigurationDetails
        """
        self._session_persistence_configuration = session_persistence_configuration

    @property
    def lb_cookie_session_persistence_configuration(self):
        """
        Gets the lb_cookie_session_persistence_configuration of this BackendSetDetails.

        :return: The lb_cookie_session_persistence_configuration of this BackendSetDetails.
        :rtype: LBCookieSessionPersistenceConfigurationDetails
        """
        return self._lb_cookie_session_persistence_configuration

    @lb_cookie_session_persistence_configuration.setter
    def lb_cookie_session_persistence_configuration(self, lb_cookie_session_persistence_configuration):
        """
        Sets the lb_cookie_session_persistence_configuration of this BackendSetDetails.

        :param lb_cookie_session_persistence_configuration: The lb_cookie_session_persistence_configuration of this BackendSetDetails.
        :type: LBCookieSessionPersistenceConfigurationDetails
        """
        self._lb_cookie_session_persistence_configuration = lb_cookie_session_persistence_configuration

    def __repr__(self):
        return formatted_flat_dict(self)

    def __eq__(self, other):
        if other is None:
            return False

        return self.__dict__ == other.__dict__

    def __ne__(self, other):
        return not self == other
