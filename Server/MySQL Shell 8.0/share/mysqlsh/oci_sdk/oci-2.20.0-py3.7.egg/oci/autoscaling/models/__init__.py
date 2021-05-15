# coding: utf-8
# Copyright (c) 2016, 2020, Oracle and/or its affiliates.  All rights reserved.
# This software is dual-licensed to you under the Universal Permissive License (UPL) 1.0 as shown at https://oss.oracle.com/licenses/upl or Apache License 2.0 as shown at http://www.apache.org/licenses/LICENSE-2.0. You may choose either license.

from __future__ import absolute_import

from .action import Action
from .auto_scaling_configuration import AutoScalingConfiguration
from .auto_scaling_configuration_summary import AutoScalingConfigurationSummary
from .auto_scaling_policy import AutoScalingPolicy
from .auto_scaling_policy_summary import AutoScalingPolicySummary
from .capacity import Capacity
from .change_auto_scaling_compartment_details import ChangeAutoScalingCompartmentDetails
from .condition import Condition
from .create_auto_scaling_configuration_details import CreateAutoScalingConfigurationDetails
from .create_auto_scaling_policy_details import CreateAutoScalingPolicyDetails
from .create_condition_details import CreateConditionDetails
from .create_scheduled_policy_details import CreateScheduledPolicyDetails
from .create_threshold_policy_details import CreateThresholdPolicyDetails
from .cron_execution_schedule import CronExecutionSchedule
from .execution_schedule import ExecutionSchedule
from .instance_pool_resource import InstancePoolResource
from .metric import Metric
from .resource import Resource
from .scheduled_policy import ScheduledPolicy
from .threshold import Threshold
from .threshold_policy import ThresholdPolicy
from .update_auto_scaling_configuration_details import UpdateAutoScalingConfigurationDetails
from .update_auto_scaling_policy_details import UpdateAutoScalingPolicyDetails
from .update_condition_details import UpdateConditionDetails
from .update_scheduled_policy_details import UpdateScheduledPolicyDetails
from .update_threshold_policy_details import UpdateThresholdPolicyDetails

# Maps type names to classes for autoscaling services.
autoscaling_type_mapping = {
    "Action": Action,
    "AutoScalingConfiguration": AutoScalingConfiguration,
    "AutoScalingConfigurationSummary": AutoScalingConfigurationSummary,
    "AutoScalingPolicy": AutoScalingPolicy,
    "AutoScalingPolicySummary": AutoScalingPolicySummary,
    "Capacity": Capacity,
    "ChangeAutoScalingCompartmentDetails": ChangeAutoScalingCompartmentDetails,
    "Condition": Condition,
    "CreateAutoScalingConfigurationDetails": CreateAutoScalingConfigurationDetails,
    "CreateAutoScalingPolicyDetails": CreateAutoScalingPolicyDetails,
    "CreateConditionDetails": CreateConditionDetails,
    "CreateScheduledPolicyDetails": CreateScheduledPolicyDetails,
    "CreateThresholdPolicyDetails": CreateThresholdPolicyDetails,
    "CronExecutionSchedule": CronExecutionSchedule,
    "ExecutionSchedule": ExecutionSchedule,
    "InstancePoolResource": InstancePoolResource,
    "Metric": Metric,
    "Resource": Resource,
    "ScheduledPolicy": ScheduledPolicy,
    "Threshold": Threshold,
    "ThresholdPolicy": ThresholdPolicy,
    "UpdateAutoScalingConfigurationDetails": UpdateAutoScalingConfigurationDetails,
    "UpdateAutoScalingPolicyDetails": UpdateAutoScalingPolicyDetails,
    "UpdateConditionDetails": UpdateConditionDetails,
    "UpdateScheduledPolicyDetails": UpdateScheduledPolicyDetails,
    "UpdateThresholdPolicyDetails": UpdateThresholdPolicyDetails
}
