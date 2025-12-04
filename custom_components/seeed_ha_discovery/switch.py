"""
Seeed HA Discovery - 开关平台
Switch platform for Seeed HA Discovery.

这个模块实现开关实体，用于：
1. 控制 LED 灯
2. 控制继电器
3. 控制其他可开关设备

工作流程：
1. 设备通过 WebSocket 发送 discovery 消息，报告其开关列表
2. 本模块根据 discovery 创建对应的开关实体
3. 用户在 HA 界面操作开关时，发送 command 消息到设备
4. 设备执行操作后，发送 state 消息确认新状态

开关数据格式示例：
{
    "id": "led",
    "name": "LED灯",
    "type": "switch",
    "icon": "mdi:lightbulb",
    "state": false
}
"""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.components.switch import SwitchEntity
from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.helpers.device_registry import DeviceInfo
from homeassistant.helpers.entity_platform import AddEntitiesCallback
from homeassistant.helpers.update_coordinator import CoordinatorEntity

from .const import DOMAIN, MANUFACTURER, CONF_DEVICE_ID, CONF_MODEL
from .coordinator import SeeedHACoordinator

# 创建日志记录器
_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(
    hass: HomeAssistant,
    entry: ConfigEntry,
    async_add_entities: AddEntitiesCallback,
) -> None:
    """
    设置开关平台
    Set up Seeed HA switches.

    这个函数在集成加载时被调用，负责：
    1. 获取设备已发现的开关
    2. 创建对应的 HA 开关实体
    3. 注册监听，动态添加新发现的开关

    参数 | Args:
        hass: Home Assistant 实例
        entry: 配置入口
        async_add_entities: 添加实体的回调函数
    """
    # 获取设备数据
    data = hass.data[DOMAIN][entry.entry_id]
    coordinator: SeeedHACoordinator = data["coordinator"]

    _LOGGER.info("设置开关平台，设备: %s", entry.data.get(CONF_DEVICE_ID))

    # 创建已发现的开关实体
    entities = []
    for entity_id, entity_config in coordinator.device.entities.items():
        # 只处理 switch 类型的实体
        if entity_config.get("type") == "switch":
            _LOGGER.info("创建开关: %s (%s)", entity_id, entity_config.get("name"))
            entities.append(SeeedHASwitch(coordinator, entity_config, entry))

    # 添加实体到 HA
    if entities:
        async_add_entities(entities)
        _LOGGER.info("已添加 %d 个开关", len(entities))

    # 注册发现回调，处理后续发现的开关
    def handle_discovery(data: dict[str, Any]) -> None:
        """
        处理新发现的开关
        Handle newly discovered switches.

        当设备报告新的开关时，动态创建实体。
        """
        new_entities = []

        for entity_id, entity_config in coordinator.device.entities.items():
            if entity_config.get("type") == "switch":
                # 检查实体是否已存在
                existing_ids = [e._entity_id for e in entities]
                if entity_id not in existing_ids:
                    _LOGGER.info("发现新开关: %s", entity_id)
                    new_entity = SeeedHASwitch(coordinator, entity_config, entry)
                    entities.append(new_entity)
                    new_entities.append(new_entity)

        if new_entities:
            async_add_entities(new_entities)
            _LOGGER.info("动态添加 %d 个新开关", len(new_entities))

    # 注册回调
    coordinator.device.add_discovery_callback(handle_discovery)


class SeeedHASwitch(CoordinatorEntity[SeeedHACoordinator], SwitchEntity):
    """
    Seeed HA 开关实体
    Representation of a Seeed HA switch.

    这个类代表一个开关实体（如 LED、继电器）。
    继承自：
    - CoordinatorEntity: 自动监听协调器的数据更新
    - SwitchEntity: HA 开关实体基类

    主要功能：
    - 显示开关状态
    - 处理开关操作
    - 发送控制命令到设备
    """

    # 使用实体名称而不是完整 ID
    _attr_has_entity_name = True

    def __init__(
        self,
        coordinator: SeeedHACoordinator,
        entity_config: dict[str, Any],
        entry: ConfigEntry,
    ) -> None:
        """
        初始化开关
        Initialize the switch.

        参数 | Args:
            coordinator: 数据协调器
            entity_config: 实体配置（来自设备发现）
                          包含: id, name, icon, state
            entry: 配置入口
        """
        # 调用父类初始化，注册到协调器
        super().__init__(coordinator)

        self._entry = entry
        self._entity_config = entity_config

        # 实体 ID（设备上报的 ID）
        self._entity_id = entity_config.get("id", "")

        # =====================================================================
        # 设置实体属性 | Set entity attributes
        # =====================================================================

        # 实体名称 - 显示在 UI 上
        self._attr_name = entity_config.get("name", self._entity_id)

        # 唯一 ID - 用于 HA 内部识别
        # 格式: {device_id}_{entity_id}
        device_id = entry.data.get(CONF_DEVICE_ID, "")
        self._attr_unique_id = f"{device_id}_{self._entity_id}"

        # 图标 - 如 mdi:lightbulb
        if icon := entity_config.get("icon"):
            self._attr_icon = icon

        _LOGGER.info(
            "开关初始化完成: %s (图标=%s)",
            self._attr_name,
            entity_config.get("icon"),
        )

    @property
    def device_info(self) -> DeviceInfo:
        """
        返回设备信息
        Return device info.

        这个属性定义了设备在 HA 设备页面的显示信息。
        同一设备的所有实体共享相同的设备信息。
        """
        device_data = self.coordinator.device.device_info
        entry_data = self._entry.data

        return DeviceInfo(
            # 设备标识符 - 用于关联实体到设备
            identifiers={(DOMAIN, entry_data.get(CONF_DEVICE_ID, ""))},
            # 设备名称
            name=device_data.get("name", "Seeed HA 设备"),
            # 制造商
            manufacturer=MANUFACTURER,
            # 设备型号
            model=entry_data.get(CONF_MODEL, device_data.get("model", "ESP32")),
            # 固件版本
            sw_version=device_data.get("version", "1.0.0"),
        )

    @property
    def available(self) -> bool:
        """
        返回实体是否可用
        Return if entity is available.

        当设备断开连接时，实体显示为不可用。
        """
        return self.coordinator.device.connected

    @property
    def is_on(self) -> bool:
        """
        返回开关的当前状态
        Return true if switch is on.

        这是开关最重要的属性，返回当前开关状态。
        值从协调器的设备数据中获取。
        """
        entities = self.coordinator.device.entities

        if self._entity_id in entities:
            state = entities[self._entity_id].get("state")
            _LOGGER.debug("开关 %s 当前状态: %s", self._entity_id, state)
            return bool(state)

        return False

    async def async_turn_on(self, **kwargs: Any) -> None:
        """
        打开开关
        Turn on the switch.

        发送 turn_on 命令到 ESP32 设备。
        """
        _LOGGER.info("发送开关命令: %s -> turn_on", self._entity_id)
        await self.coordinator.device.async_send_command(
            self._entity_id,
            command="turn_on"
        )

    async def async_turn_off(self, **kwargs: Any) -> None:
        """
        关闭开关
        Turn off the switch.

        发送 turn_off 命令到 ESP32 设备。
        """
        _LOGGER.info("发送开关命令: %s -> turn_off", self._entity_id)
        await self.coordinator.device.async_send_command(
            self._entity_id,
            command="turn_off"
        )

    async def async_toggle(self, **kwargs: Any) -> None:
        """
        切换开关状态
        Toggle the switch.

        发送 toggle 命令到 ESP32 设备。
        """
        _LOGGER.info("发送开关命令: %s -> toggle", self._entity_id)
        await self.coordinator.device.async_send_command(
            self._entity_id,
            command="toggle"
        )

    @property
    def extra_state_attributes(self) -> dict[str, Any]:
        """
        返回额外的状态属性
        Return extra state attributes.

        这些属性会显示在实体的属性面板中。
        """
        entities = self.coordinator.device.entities

        if self._entity_id in entities:
            return entities[self._entity_id].get("attributes", {})

        return {}

