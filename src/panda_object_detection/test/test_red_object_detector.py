#!/usr/bin/env python3
import numpy as np
import pytest
import rclpy
import cv2
from panda_object_detection.red_object_detection import RedObjectDetector
from sensor_msgs.msg import CameraInfo

def test_valid_depth():
    rclpy.init()
    node = RedObjectDetector()
    depth_image = np.ones((480, 640), dtype=np.float32) * 1.5
    depth = node.get_depth_at_pixel(320, 240, depth_image)
    assert depth == 1.5
    rclpy.shutdown()

def test_out_of_bounds():
    rclpy.init()
    node = RedObjectDetector()
    depth_image = np.ones((480, 640), dtype=np.float32)
    depth = node.get_depth_at_pixel(1000, 1000, depth_image)
    assert depth is None
    rclpy.shutdown()

def test_nan_depth():
    rclpy.init()
    node = RedObjectDetector()
    depth_image = np.ones((480, 640), dtype=np.float32)
    depth_image[240, 320] = np.nan
    depth = node.get_depth_at_pixel(320, 240, depth_image)
    assert depth is None
    rclpy.shutdown()

def test_zero_depth():
    rclpy.init()
    node = RedObjectDetector()
    depth_image = np.ones((480, 640), dtype=np.float32)
    depth_image[240, 320] = 0.0
    depth = node.get_depth_at_pixel(320, 240, depth_image)
    assert depth is None
    rclpy.shutdown()

def test_find_red_object_centroid():
    rclpy.init()
    node = RedObjectDetector()
    image = np.zeros((480, 640, 3), dtype=np.uint8)
    cv2.circle(image, (320, 240), 50, (0, 0, 255), -1)
    cX, cY = node.find_red_object_centroid(image)
    assert cX is not None and cY is not None
    assert abs(cX - 320) < 10
    assert abs(cY - 240) < 10
    rclpy.shutdown()

def test_pixel_to_camera_coords_center():
    rclpy.init()
    node = RedObjectDetector()
    camera_info = CameraInfo()
    camera_info.k = [600.0, 0.0, 320.0, 0.0, 600.0, 240.0, 0.0, 0.0, 1.0]  # fx, fy, cx, cy
    u, v = 320, 240  # center pixel
    depth = 1.0

    X, Y, Z = node.pixel_to_camera_coords(u, v, depth, camera_info)
    assert X == 0
    assert Y == 0
    assert Z == 1.0
    rclpy.shutdown()

def test_pixel_to_camera_coords_offset():
    rclpy.init()
    node = RedObjectDetector()
    camera_info = CameraInfo()
    camera_info.k = [600.0, 0.0, 320.0, 0.0, 600.0, 240.0, 0.0, 0.0, 1.0]
    u, v = 340, 260  # offset pixel
    depth = 2.0

    X, Y, Z = node.pixel_to_camera_coords(u, v, depth, camera_info)
    assert round(X, 2) == round(((340-320) * 2.0 / 600.0), 2)
    assert round(Y, 2) == round(((260-240) * 2.0 / 600.0), 2)
    assert Z == 2.0
    rclpy.shutdown()

def test_detect_red_object_3d():
    rclpy.init()
    node = RedObjectDetector()
    rgb_image = np.zeros((480, 640, 3), dtype=np.uint8)
    cv2.circle(rgb_image, (320, 240), 30, (0, 0, 255), -1)
    depth_image = np.ones((480, 640), dtype=np.float32) * 1.5

    camera_info = CameraInfo()
    camera_info.k = [600.0, 0.0, 320.0, 0.0, 600.0, 240.0, 0.0, 0.0, 1.0]
    cX, cY = node.find_red_object_centroid(rgb_image)
    depth = node.get_depth_at_pixel(cX, cY, depth_image)
    result = node.pixel_to_camera_coords(cX, cY, depth, camera_info)

    assert result is not None
    X, Y, Z = result
    assert abs(X) < 1e-6
    assert abs(Y) < 1e-6
    assert Z == 1.5
    rclpy.shutdown()