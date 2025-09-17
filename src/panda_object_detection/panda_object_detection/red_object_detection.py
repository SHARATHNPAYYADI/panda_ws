import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image, CameraInfo
from cv_bridge import CvBridge
import cv2
import numpy as np
import tf2_ros
import numpy as np
np.float = float
import tf_transformations
from geometry_msgs.msg import PointStamped
from visualization_msgs.msg import Marker
from panda_msgs.srv import DetectObject

class RedObjectDetector(Node):
    def __init__(self):
        super().__init__('red_object_detector')
        self.bridge = CvBridge()
        self.camera_info = None
        self.depth_image = None
        self.latest_rgb_image = None


        # Subscribers
        self.create_subscription(CameraInfo, '/camera1/camera_info', self.camera_info_callback, 10)
        self.create_subscription(Image, '/camera1/image_raw', self.image_callback, 10)
        self.create_subscription(Image, '/camera1/depth/image_raw', self.depth_callback, 10)
        self.marker_pub = self.create_publisher(Marker, '/red_object_marker', 10)
        self.orig_marker_pub = self.create_publisher(Marker, '/orig_object_marker', 10)

        #Services
        self.srv = self.create_service(DetectObject, '/detect_objects', self.detect_callback)

        # TF Buffer and Listener
        self.tf_buffer = tf2_ros.Buffer()
        self.tf_listener = tf2_ros.TransformListener(self.tf_buffer, self)

    def camera_info_callback(self, msg):
        self.camera_info = msg

    def depth_callback(self, msg):
        self.depth_image = msg
    
    def image_callback(self,msg):
        self.latest_rgb_image = msg

    def detect_callback(self, request, responce):
        """
        Calculates the position of red object and provide the reply for service

        Args:
            request: trigger to detect the camera
        Returns:
            responce:  Consists of position of object X,Y, Z format
        """
        if self.camera_info is None or self.depth_image is None:
            return

        # Convert images
        cv_image = self.bridge.imgmsg_to_cv2(self.latest_rgb_image, desired_encoding='bgr8')
        depth_image = self.bridge.imgmsg_to_cv2(self.depth_image, desired_encoding='passthrough')
        #find centroid
        centroid = self.find_red_object_centroid(cv_image)
        if centroid is None:
            responce.success = False
            responce.message = "Object not detected"
            return responce
        cX, cY = centroid
        # Get depth at centroid
        depth = self.get_depth_at_pixel(cX, cY, depth_image)
        if depth is None:
            return

        #object with respect to camera co-orinates
        X, Y, Z = self.pixel_to_camera_coords(cX, cY, depth, self.camera_info)

        self.get_logger().info(f"Object position in camera frame: {X}, {Y}, {Z}")
        marker = Marker()
        marker.header.frame_id = "camera_depth_optical_frame" #'world'
        marker.header.stamp = self.get_clock().now().to_msg()
        marker.ns = 'camera_reference'
        marker.id = 0
        marker.type = Marker.SPHERE
        marker.action = Marker.ADD
        marker.pose.position.x = X
        marker.pose.position.y = Y
        marker.pose.position.z = Z
        marker.pose.orientation.w = 1.0
        marker.scale.x = 0.05
        marker.scale.y = 0.05
        marker.scale.z = 0.05
        marker.color.r = 1.0
        marker.color.g = 0.0
        marker.color.b = 0.0
        marker.color.a = 1.0
        self.marker_pub.publish(marker)
        # Transform to world frame
        try:
            trans = self.tf_buffer.lookup_transform('world', 'camera_depth_optical_frame', rclpy.time.Time())
            # Compose point in camera frame
            
            point_camera = np.array([X, Y, Z, 1])
            
            point_world = self.transform_point_to_world(point_camera, trans)

            responce.x = point_world[0]
            responce.y = point_world[1]
            responce.z = point_world[2]
            responce.success = True
            responce.message = "Red object Detected"

            self.get_logger().info(f"Object position in world frame: {point_world[:3]}")
            orig_marker = Marker()
            orig_marker.header.frame_id = "world"
            orig_marker.header.stamp = self.get_clock().now().to_msg()
            orig_marker.ns = 'World reference'
            orig_marker.id = 0
            orig_marker.type = Marker.SPHERE
            orig_marker.action = Marker.ADD
            orig_marker.pose.position.x = point_world[0]
            orig_marker.pose.position.y = point_world[1]
            orig_marker.pose.position.z = point_world[2]
            orig_marker.pose.orientation.w = 1.0
            orig_marker.scale.x = 0.05
            orig_marker.scale.y = 0.05
            orig_marker.scale.z = 0.05
            orig_marker.color.r = 1.0
            orig_marker.color.g = 0.0
            orig_marker.color.b = 0.0
            orig_marker.color.a = 1.0
            self.orig_marker_pub.publish(orig_marker)
        except Exception as e:
            self.get_logger().error(f"Transform error: {e}")
            responce.success = False
            responce.message = "Red object Not Detected"
        return responce

    def get_depth_at_pixel(self, u, v, depth_image):
        # Handle depth image indexing
        if v >= depth_image.shape[0] or u >= depth_image.shape[1]:
            return None
        depth_value = depth_image[v, u]
        if np.isnan(depth_value) or depth_value <= 0:
            return None
        return float(depth_value)
    
    def pixel_to_camera_coords(self, u, v, depth, camera_info):
        # Camera intrinsics
        K = camera_info.k  # [fx, 0, cx, 0, fy, cy, 0, 0, 1]
        fx = K[0]
        fy = K[4]
        cx = K[2]
        cy = K[5]

        # Camera pixel to camera co-ordinates
        X = (u - cx) * depth / fx
        Y = (v - cy) * depth / fy
        Z = depth
        return X, Y, Z

    def transform_point_to_world(self, point_camera, transform):
        # Tranforming from camera to world format
        t = transform.transform.translation
        r = transform.transform.rotation

        T = tf_transformations.translation_matrix([t.x, t.y, t.z])
        R = tf_transformations.quaternion_matrix([r.x, r.y, r.z, r.w])

        # point_camera_hom = np.array([*point_camera, 1.0])  # [X, Y, Z, 1]
        point_world = np.dot(T, np.dot(R, point_camera))
        return point_world[:3]

    def find_red_object_centroid(self, cv_image):
        # Convert to HSV
        hsv = cv2.cvtColor(cv_image, cv2.COLOR_BGR2HSV)
        # Red color range
        lower_red1 = np.array([0, 100, 100])
        upper_red1 = np.array([10, 255, 255])
        lower_red2 = np.array([160, 100, 100])
        upper_red2 = np.array([179, 255, 255])
        # Find contours
        mask1 = cv2.inRange(hsv, lower_red1, upper_red1)
        mask2 = cv2.inRange(hsv, lower_red2, upper_red2)
        mask = cv2.bitwise_or(mask1, mask2)

        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        if contours:
            c = max(contours, key=cv2.contourArea)
            M = cv2.moments(c)
            if M["m00"] > 0:
                cX = int(M["m10"] / M["m00"])
                cY = int(M["m01"] / M["m00"])
                return (cX, cY)
        return None

def main(args=None):
    rclpy.init(args=args)
    node = RedObjectDetector()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
    main()