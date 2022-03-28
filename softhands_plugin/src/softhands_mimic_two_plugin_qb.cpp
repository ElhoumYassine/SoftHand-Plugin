#include <softhands_plugin/softhands_mimic_two_plugin_qb.h>
#include <gazebo/common/Plugin.hh>
#include <gazebo/common/common.hh>
#include <gazebo/physics/physics.hh>
#include <boost/bind.hpp>
#include <math.h>
#include <string>
#include <ros/ros.h>

using namespace gazebo;
using namespace std;

/*


*/

void softhandsPluginTwoMimicQB::Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf)
{
  int argc = 0;
  char **argv;
  ros::init(argc, argv, "softhands_Two_Plugin_QB");

  this->model = _parent;

  // Make sure the ROS node for Gazebo has already been initialized                                                                                    
  if (!ros::isInitialized())
  {
    ROS_FATAL_STREAM("A ROS node for Gazebo has not been initialized, unable to load plugin. "
      << "Load the Gazebo system plugin 'libgazebo_ros_api_plugin.so' in the gazebo_ros package)");
    return;
  }

  // Retrieve joint identifier and control mode from urdf tags
  this->joint_name =_sdf->GetElement("joint")->Get<string>();
  this->ns_name =_sdf->GetElement("namespace")->Get<string>();
  this->K =_sdf->GetElement("stiffness")->Get<double>();

  // Everything-is-fine message
  std::string ok_msg = "Softhands mimic two plugin on " + joint_name + " started!";
  ROS_WARN_STREAM(ok_msg);

  // Retrieve joints
  this->joint = this->model->GetJoint(joint_name);

  // Compose the name of the subscriber, removing the namespace in order to be coherent with the synergy plugin
  int len_ns = ns_name.length();
  cmd_sub_name = ns_name + "/" + joint_name + "/command";

std::cout << joint_name << "\t" << std::endl;

  int len_name = joint_name.length();
  std::string joint_name_w_mimic = joint_name.erase(len_name-6, 6) + "_virtual_joint";
  this->joint_mimic = this->model->GetJoint(joint_name_w_mimic);

  std::cout << joint_name_w_mimic << std::endl;

  // Subscribers and Publishers for the joint states and command
  sub = n.subscribe(cmd_sub_name, 10, &softhandsPluginTwoMimicQB::getRef_callback, this);
  //pub = n.advertise<std_msgs::Float64>(cmd_pub_name, 500);

  this->updateConnection = event::Events::ConnectWorldUpdateBegin(boost::bind(&softhandsPluginTwoMimicQB::OnUpdate, this, _1));  

    // DEBUG
/*  std::cout << cmd_ref1_name << std::endl;
  std::cout << cmd_ref2_name << std::endl;*/
}

// Subscriber callbacks references
void softhandsPluginTwoMimicQB::getRef_callback(const std_msgs::Float64& val)
{
    this->joint_ref = val;
}

// Main update function
void softhandsPluginTwoMimicQB::OnUpdate(const common::UpdateInfo & /*_info*/)
{
  // Retrieve joint actual position
  this->q = this->joint->GetAngle(0).Radian();
  this->dq = this->joint->GetVelocity(0);
  this->q_mimic = this->joint_mimic->GetAngle(0).Radian();
  this->dq_mimic = this->joint_mimic->GetVelocity(0);

  // Set to the joint elastic torque
/*  this->joint->SetForce(0, this->K*(this->joint_ref.data - this->q) + 2*this->K*(this->q_mimic - this->q) - this->Damp*this->dq);
  this->joint_mimic->SetForce(0, this->K*(this->joint_ref.data - this->q_mimic) - 2*this->K*(this->q_mimic - this->q) - this->Damp*this->dq_mimic);
*/
   this->joint->SetForce(0, this->K*(this->joint_ref.data - this->q) - this->Damp*this->dq);
  this->joint_mimic->SetForce(0, this->K*(this->joint_ref.data - this->q_mimic) - this->Damp*this->dq_mimic);
 
  //pub.publish(this->joint_tau);
}