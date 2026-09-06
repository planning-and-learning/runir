"""Serialize Runir values into shared Runir and Tyr reference tables."""

from pytyr.serialization import Dictionaries as Dictionaries
from pyyggdrasil.serialization import JSONValue as JSONValue, Row as Row

from .._pyrunir.serialization import register_table as register_table, serialize as serialize, table as table
