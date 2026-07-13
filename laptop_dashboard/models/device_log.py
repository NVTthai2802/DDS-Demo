from dataclasses import dataclass
from cyclonedds.idl import IdlStruct
import cyclonedds.idl.types as types

@dataclass
class DeviceLog(IdlStruct, typename="DeviceLog"):
    device_id: str
    device_type: str
    temperature: types.float64
    humidity: types.float64
    co2: types.int32
    light: types.int32
    occupancy: bool
    battery: types.int32
    signal_strength: types.int32
    timestamp: types.float64
    sequence_number: types.int32
