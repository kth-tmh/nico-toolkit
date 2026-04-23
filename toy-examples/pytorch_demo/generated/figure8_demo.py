#!/usr/bin/env python3
from __future__ import annotations

import argparse
import math
import pathlib
from typing import Dict, List

import torch


SPEC = {
  "connections": [
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "trig_1",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_1",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_2",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_3",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_4",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_5",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_6",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_7",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_8",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_9",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 1,
      "from": "hidden_10",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_1",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_2",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_3",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_4",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_5",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_6",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_7",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_8",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_9",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 2,
      "from": "hidden_10",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_1",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_2",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_3",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_4",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_5",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_6",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_7",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_8",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_9",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 3,
      "from": "hidden_10",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_1",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_2",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_3",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_4",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_5",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_6",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_7",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_8",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_9",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 4,
      "from": "hidden_10",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_1",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_2",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_3",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_4",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_5",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_6",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_7",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_8",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_9",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_1"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_2"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_3"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_4"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_5"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_6"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_7"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_8"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_9"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 5,
      "from": "hidden_10",
      "to": "hidden_10"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_1",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_1",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_2",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_2",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_3",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_3",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_4",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_4",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_5",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_5",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_6",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_6",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_7",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_7",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_8",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_8",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_9",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_9",
      "to": "y"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_10",
      "to": "x"
    },
    {
      "bound": 0.1,
      "const_weight": null,
      "delay": 0,
      "from": "hidden_10",
      "to": "y"
    }
  ],
  "default_basenames": [
    "fig8"
  ],
  "example_dir": "Figure8",
  "example_name": "figure8",
  "notes": [
    "NormStream -c 0 1 -s POS net.rtdnn.gz",
    "BackProp -p net.log -g1e-2 -E -i400 -d net.rtdnn.gz fig8.trig"
  ],
  "script_path": "toy-examples/Figure8/mknet",
  "streams": {
    "POS": {
      "directory": ".",
      "ext": "pos",
      "linked_units": [
        "x",
        "y"
      ],
      "mode": "t",
      "name": "POS",
      "size": 2
    },
    "TRIG": {
      "directory": ".",
      "ext": "trig",
      "linked_units": [
        "trig_1"
      ],
      "mode": "r",
      "name": "TRIG",
      "size": 1
    }
  },
  "units": [
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "trig_1",
      "role": "input"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "x",
      "role": "output"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "y",
      "role": "output"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_1",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_2",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_3",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_4",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_5",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_6",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_7",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_8",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_9",
      "role": "hidden"
    },
    {
      "activation": "tanh",
      "bias_bound": 0.1,
      "name": "hidden_10",
      "role": "hidden"
    }
  ]
}
ROOT = pathlib.Path(__file__).resolve().parents[3]
EXAMPLE_DIR = ROOT / "toy-examples" / SPEC["example_dir"]


def read_ascii_matrix(path: pathlib.Path, width: int) -> torch.Tensor:
    rows: List[List[float]] = []
    for raw in path.read_text().splitlines():
        line = raw.strip()
        if not line:
            continue
        values = [float(part) for part in line.split()]
        if len(values) != width:
            raise ValueError(f"Expected {width} columns in {path}, got {len(values)}")
        rows.append(values)
    return torch.tensor(rows, dtype=torch.float32)


def stream_file(base_name: str, stream_name: str) -> pathlib.Path:
    stream = SPEC["streams"][stream_name]
    directory = EXAMPLE_DIR / stream["directory"]
    return directory / f"{base_name}.{stream['ext']}"


def load_example(base_name: str) -> tuple[Dict[str, torch.Tensor], Dict[str, torch.Tensor]]:
    inputs: Dict[str, torch.Tensor] = {}
    targets: Dict[str, torch.Tensor] = {}
    for name, stream in SPEC["streams"].items():
        tensor = read_ascii_matrix(stream_file(base_name, name), stream["size"])
        if stream["mode"] == "r":
            inputs[name] = tensor
        else:
            targets[name] = tensor
    return inputs, targets


def activation(kind: str, value: torch.Tensor) -> torch.Tensor:
    if kind == "linear":
        return value
    if kind == "tanh":
        return torch.tanh(value)
    if kind == "sigmoid":
        return torch.sigmoid(value)
    if kind == "arctan":
        return torch.atan(value)
    raise ValueError(f"Unsupported activation: {kind}")


class ToyNICOModel(torch.nn.Module):
    def __init__(self) -> None:
        super().__init__()
        self.num_units = len(SPEC["units"])
        self.unit_names = [unit["name"] for unit in SPEC["units"]]
        self.activations = [unit["activation"] for unit in SPEC["units"]]
        self.roles = [unit["role"] for unit in SPEC["units"]]
        self.unit_index = {name: index for index, name in enumerate(self.unit_names)}
        self.edge_from = [self.unit_index[edge["from"]] for edge in SPEC["connections"]]
        self.edge_to = [self.unit_index[edge["to"]] for edge in SPEC["connections"]]
        self.edge_delay = [int(edge["delay"]) for edge in SPEC["connections"]]
        self.incoming = {index: [] for index in range(self.num_units)}
        for edge_index, unit_index in enumerate(self.edge_to):
            self.incoming[unit_index].append(edge_index)
        self.output_links = {
            name: [self.unit_index[unit_name] for unit_name in stream["linked_units"]]
            for name, stream in SPEC["streams"].items()
            if stream["mode"] != "r"
        }
        self.input_links = {
            name: [self.unit_index[unit_name] for unit_name in stream["linked_units"]]
            for name, stream in SPEC["streams"].items()
            if stream["mode"] == "r"
        }
        self.edge_weight = torch.nn.Parameter(torch.empty(len(self.edge_from), dtype=torch.float32))
        self.bias = torch.nn.Parameter(torch.zeros(self.num_units, dtype=torch.float32))
        self.reset_parameters()

    def reset_parameters(self) -> None:
        with torch.no_grad():
            for index, edge in enumerate(SPEC["connections"]):
                const_weight = edge["const_weight"]
                if const_weight is not None:
                    self.edge_weight[index] = float(const_weight)
                else:
                    bound = float(edge["bound"])
                    self.edge_weight[index].uniform_(-bound, bound)
            self.bias.zero_()
            for index, unit in enumerate(SPEC["units"]):
                if unit["role"] == "input":
                    continue
                bound = float(unit["bias_bound"])
                if bound > 0.0:
                    self.bias[index].uniform_(-bound, bound)

    def forward(self, inputs: Dict[str, torch.Tensor]) -> Dict[str, torch.Tensor]:
        first_stream = next(iter(inputs.values()))
        steps = int(first_stream.shape[0])
        cache: Dict[tuple[int, int], torch.Tensor] = {}
        input_positions = {}
        for stream_name, unit_indexes in self.input_links.items():
            for offset, unit_index in enumerate(unit_indexes):
                input_positions[unit_index] = (stream_name, offset)

        def unit_value(unit_index: int, step: int) -> torch.Tensor:
            if step < 0 or step >= steps:
                return self.edge_weight.new_zeros(())
            key = (unit_index, step)
            if key in cache:
                return cache[key]
            if self.roles[unit_index] == "input":
                stream_name, offset = input_positions[unit_index]
                value = inputs[stream_name][step][offset]
                cache[key] = value
                return value
            total = self.bias[unit_index]
            for edge_index in self.incoming[unit_index]:
                source_index = self.edge_from[edge_index]
                source_step = step - self.edge_delay[edge_index]
                total = total + self.edge_weight[edge_index] * unit_value(source_index, source_step)
            value = activation(self.activations[unit_index], total)
            cache[key] = value
            return value

        return {
            stream_name: torch.stack(
                [
                    torch.stack([unit_value(unit_index, step) for unit_index in unit_indexes], dim=0)
                    for step in range(steps)
                ],
                dim=0,
            )
            for stream_name, unit_indexes in self.output_links.items()
        }


def loss_for_outputs(outputs: Dict[str, torch.Tensor], targets: Dict[str, torch.Tensor]) -> torch.Tensor:
    total = torch.tensor(0.0, dtype=torch.float32)
    for stream_name, target in targets.items():
        total = total + torch.nn.functional.mse_loss(outputs[stream_name], target)
    return total


def train_model(model: ToyNICOModel, basenames: List[str], epochs: int, lr: float) -> None:
    optimizer = torch.optim.Adam(model.parameters(), lr=lr)
    for epoch in range(epochs):
        epoch_loss = 0.0
        for base_name in basenames:
            inputs, targets = load_example(base_name)
            optimizer.zero_grad()
            outputs = model(inputs)
            loss = loss_for_outputs(outputs, targets)
            loss.backward()
            optimizer.step()
            epoch_loss += float(loss.detach())
        if epoch % max(1, epochs // 10) == 0 or epoch == epochs - 1:
            print(f"epoch={epoch:4d} loss={epoch_loss:.6f}")


def polyline(points: List[tuple[float, float]], color: str, width: float) -> str:
    joined = " ".join(f"{x:.2f},{y:.2f}" for x, y in points)
    return f'<polyline fill="none" stroke="{color}" stroke-width="{width}" points="{joined}" />'


def scale_xy(values: torch.Tensor, width: int, height: int, padding: int) -> List[tuple[float, float]]:
    xs = values[:, 0]
    ys = values[:, 1]
    min_x = float(xs.min())
    max_x = float(xs.max())
    min_y = float(ys.min())
    max_y = float(ys.max())
    span_x = max(max_x - min_x, 1e-6)
    span_y = max(max_y - min_y, 1e-6)
    points: List[tuple[float, float]] = []
    for x_value, y_value in zip(xs.tolist(), ys.tolist()):
        x = padding + (x_value - min_x) / span_x * (width - 2 * padding)
        y = height - padding - (y_value - min_y) / span_y * (height - 2 * padding)
        points.append((x, y))
    return points


def scale_series(values: torch.Tensor, width: int, height: int, padding: int, offset: float) -> List[tuple[float, float]]:
    min_y = float(values.min())
    max_y = float(values.max())
    span_y = max(max_y - min_y, 1e-6)
    steps = max(1, values.shape[0] - 1)
    points: List[tuple[float, float]] = []
    for index, value in enumerate(values.tolist()):
        x = padding + index / steps * (width - 2 * padding)
        y = height - padding - (value - min_y) / span_y * (height - 2 * padding) + offset
        points.append((x, y))
    return points


def save_svg(outputs: Dict[str, torch.Tensor], targets: Dict[str, torch.Tensor], path: pathlib.Path) -> None:
    stream_name = next(iter(targets.keys()))
    prediction = outputs[stream_name].detach().cpu()
    target = targets[stream_name].detach().cpu()
    width = 900
    height = 420
    padding = 40
    lines: List[str] = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="#fffaf2" />',
        '<text x="40" y="28" font-family="Helvetica, Arial, sans-serif" font-size="20">NICO toy-example prediction vs target</text>',
        '<text x="40" y="50" font-family="Helvetica, Arial, sans-serif" font-size="12" fill="#555">orange = target, blue = prediction</text>',
    ]
    if target.shape[1] == 2:
        lines.append(polyline(scale_xy(target, width, height, padding), "#d97706", 3.0))
        lines.append(polyline(scale_xy(prediction, width, height, padding), "#2563eb", 2.0))
    else:
        colors = ["#d97706", "#059669", "#7c3aed", "#dc2626"]
        for index in range(target.shape[1]):
            lines.append(polyline(scale_series(target[:, index], width, height, padding, 0.0), colors[index % len(colors)], 3.0))
            lines.append(polyline(scale_series(prediction[:, index], width, height, padding, 0.0), "#2563eb", 1.5))
    lines.append("</svg>")
    path.write_text("\n".join(lines))


def report_outputs(model: ToyNICOModel, basenames: List[str], svg_path: str | None) -> None:
    for base_name in basenames:
        inputs, targets = load_example(base_name)
        outputs = model(inputs)
        print(f"example={base_name}")
        for stream_name, values in outputs.items():
            print(stream_name, values.detach())
        if svg_path:
            output_path = pathlib.Path(svg_path)
            if len(basenames) > 1:
                output_path = output_path.with_name(f"{output_path.stem}_{base_name}{output_path.suffix}")
            save_svg(outputs, targets, output_path)
            print(f"wrote {output_path}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Run a generated PyTorch demo for a NICO toy example.")
    parser.add_argument("--epochs", type=int, default=600, help="training epochs")
    parser.add_argument("--lr", type=float, default=0.01, help="optimizer learning rate")
    parser.add_argument("--seed", type=int, default=0, help="random seed")
    parser.add_argument("--skip-train", action="store_true", help="skip training and only run inference")
    parser.add_argument("--basename", action="append", default=[], help="example basename to load")
    parser.add_argument("--svg", default="figure8.svg", help="SVG output path")
    args = parser.parse_args()

    torch.manual_seed(args.seed)
    basenames = args.basename or SPEC["default_basenames"]
    model = ToyNICOModel()
    if not args.skip_train:
        train_model(model, basenames, epochs=args.epochs, lr=args.lr)
    report_outputs(model, basenames, svg_path=args.svg)


if __name__ == "__main__":
    main()
