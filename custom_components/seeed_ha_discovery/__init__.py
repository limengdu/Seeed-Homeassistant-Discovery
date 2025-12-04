"""
Seeed HA Discovery - 主入口文件
The Seeed HA Discovery integration.

这个文件是 Home Assistant 集成的主入口，负责：
1. 初始化集成 - 当用户添加设备时调用
2. 建立与 ESP32 设备的 WebSocket 连接
3. 加载传感器平台
4. 处理集成的卸载

工作流程：
1. 用户通过配置流程添加设备（手动输入IP或自动发现）
2. async_setup_entry() 被调用，创建设备连接
3. 加载 sensor 平台，创建传感器实体
4. 保持 WebSocket 连接，实时接收传感器数据
"""
from __future__ import annotations

import logging
from typing import Any

from homeassistant.config_entries import ConfigEntry
from homeassistant.core import HomeAssistant
from homeassistant.exceptions import ConfigEntryNotReady

from .const import (
    DOMAIN,
    PLATFORMS,
    CONF_HOST,
    CONF_PORT,
    DEFAULT_WS_PORT,
)
from .coordinator import SeeedHACoordinator
from .device import SeeedHADevice

# 创建日志记录器
# Create logger for this module
_LOGGER = logging.getLogger(__name__)


async def async_setup_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """
    设置配置入口
    Set up Seeed HA Discovery from a config entry.

    当用户成功添加一个设备后，这个函数会被调用。
    This function is called when a user successfully adds a device.

    参数 | Args:
        hass: Home Assistant 实例 | Home Assistant instance
        entry: 配置入口，包含设备信息 | Config entry containing device info

    返回 | Returns:
        bool: 设置是否成功 | Whether setup was successful
    """
    # 确保存储字典存在
    # Ensure the storage dictionary exists
    hass.data.setdefault(DOMAIN, {})

    # 从配置中获取设备地址和端口
    # Get device address and port from config
    host = entry.data[CONF_HOST]
    port = entry.data.get(CONF_PORT, DEFAULT_WS_PORT)

    _LOGGER.info("正在设置 Seeed HA 设备: %s:%s", host, port)
    _LOGGER.info("Setting up Seeed HA device: %s:%s", host, port)

    # 创建设备实例 - 负责与 ESP32 通信
    # Create device instance - handles communication with ESP32
    device = SeeedHADevice(hass, host, port, entry)

    # 创建协调器 - 管理数据更新和实体状态
    # Create coordinator - manages data updates and entity states
    coordinator = SeeedHACoordinator(hass, device, entry)

    # 尝试连接到设备
    # Try to connect to the device
    try:
        await coordinator.async_connect()
        _LOGGER.info("成功连接到设备 %s", host)
        _LOGGER.info("Successfully connected to device %s", host)
    except Exception as err:
        _LOGGER.error("无法连接到 Seeed HA 设备 %s: %s", host, err)
        _LOGGER.error("Failed to connect to Seeed HA device at %s: %s", host, err)
        # 抛出异常，HA 会稍后重试
        # Raise exception, HA will retry later
        raise ConfigEntryNotReady(f"无法连接到 {host}") from err

    # 保存设备和协调器引用，供其他模块使用
    # Store device and coordinator references for other modules
    hass.data[DOMAIN][entry.entry_id] = {
        "device": device,
        "coordinator": coordinator,
    }

    # 加载传感器平台
    # Load sensor platform
    await hass.config_entries.async_forward_entry_setups(entry, PLATFORMS)

    # 注册配置更新监听器
    # Register config update listener
    entry.async_on_unload(entry.add_update_listener(async_update_options))

    return True


async def async_unload_entry(hass: HomeAssistant, entry: ConfigEntry) -> bool:
    """
    卸载配置入口
    Unload a config entry.

    当用户删除设备或重新加载集成时调用。
    Called when user removes device or reloads the integration.

    参数 | Args:
        hass: Home Assistant 实例
        entry: 要卸载的配置入口

    返回 | Returns:
        bool: 卸载是否成功
    """
    _LOGGER.info("正在卸载 Seeed HA 设备")
    _LOGGER.info("Unloading Seeed HA device")

    # 卸载所有平台
    # Unload all platforms
    unload_ok = await hass.config_entries.async_unload_platforms(entry, PLATFORMS)

    if unload_ok:
        # 获取并清理设备数据
        # Get and clean up device data
        data = hass.data[DOMAIN].pop(entry.entry_id)
        coordinator: SeeedHACoordinator = data["coordinator"]

        # 断开 WebSocket 连接
        # Disconnect WebSocket connection
        await coordinator.async_disconnect()

        _LOGGER.info("设备卸载完成")
        _LOGGER.info("Device unloaded successfully")

    return unload_ok


async def async_update_options(hass: HomeAssistant, entry: ConfigEntry) -> None:
    """
    处理配置选项更新
    Handle options update.

    当用户修改设备配置时调用，会重新加载集成。
    Called when user modifies device config, will reload the integration.
    """
    _LOGGER.info("配置已更新，正在重新加载")
    _LOGGER.info("Config updated, reloading")
    await hass.config_entries.async_reload(entry.entry_id)
