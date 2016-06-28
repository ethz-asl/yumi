struct planner {
	std::vector<planningInterface::MoveGroup::Plan> plans; // include vector to store planned paths for trajectory points
	std::string groupName; // include group name
	bool success; // include boolean to store whether the planner was successful or not
};

struct trajectoryJoints_point {
	std::vector<double> joint;
};

struct trajectoryJoints {
	std::string groupName; // include group name
	std::vector<trajectoryJoints_point> joints;
	int totalJoints;
};

struct trajectoryPoses {
	std::string groupName;
	std::vector<geometry_msgs::Pose> pose_left;
	std::vector<geometry_msgs::Pose> pose_right;
};

planner generatePathsAndExecute(std::string, planningInterface::MoveGroup&);
planner generatePaths(std::string, planningInterface::MoveGroup&);
bool executePaths(planningInterface::MoveGroup&, planner&);

planningInterface::MoveGroup::Plan getTrajectory(planningInterface::MoveGroup&, std::string);
trajectoryJoints getTrajectoryJoints(planningInterface::MoveGroup&, std::string);
trajectoryPoses getTrajectoryPoses(planningInterface::MoveGroup&, std::string);


planningInterface::MoveGroup::Plan getTrajectory(planningInterface::MoveGroup& group, std::string inputFile) {
/*
	NOTE: Need to figure out how to create joint trajectory? Maybe conver to pose first? | DATE: 2016-06-28
*/
	trajectoryPoses poseTrajectory = getTrajectoryPoses(group,inputFile);
	moveit_msgs::RobotTrajectory trajectory;
	ROS_INFO("Computing cartesian path.");
	double fraction = group.computeCartesianPath(poseTrajectory,0.01,0.0,trajectory); // what is fraction?
	ROS_INFO("Successful computation of cartesian path.");

	ROS_INFO("Creating plan.");
	planningInterface::MoveGroup::Plan plan;
	plan.trajectory_ = trajectory;
	ROS_INFO("Plan successfully created. Total points: %lu",plan.trajectory_.joint_trajectory.points.size());

	return plan;
}

trajectoryJoints getTrajectoryJoints(planningInterface::MoveGroup& group, std::string inputFile) {
/*
	NOTE: Should push back be used below for storing joint values? | DATE 2016-06-28
*/
	// Initialize Variables
	std::string line; // variable to store each line of text from the input file
	std::string command; // variable to store the command for each trajectory point from the input file
	int currentLine = 0;

	std::string groupName_1; // variable to store the first group name
	std::string groupName_2; // variable to store the second group name
	std::vector<double>::size_type totalJoints_1; // variable to store the joint values for the first group
	std::vector<double>::size_type totalJoints_2; // variable to store the joint values for the second group

	std::string groupName = group.getName();
	trajectoryJoints trajectory_joints;
	trajectory_joints.groupName = groupName;

	// Get Group Name
	int groupName_index;
	if (groupName.compare("left_arm") == 0) {
		groupName_index = 1;
	} else if (groupName.comapre("right_arm") == 0) {
		groupName_index = 2;
	} else if (groupName.comapre("both_arms") == 0) {
		groupName_index = 3;
	} else {
		groupName_index = -1;
		ROS_ERROR("Provided group name is not recognized.");
		ROS_INFO("Recognized group names list: left_arm, right_arm, both_arms");
		ROS_INFO("Provided group: %s",groupName.c_str());
	}

	// Notfify User that the Planner is About to Start
	ROS_INFO("Getting stored trajectory joints from file."); // notify user that the planner is about to start
	ROS_INFO("File location: %s",inputFile.c_str()); // notify user of the full path of the input file

	// Open File and Generate Plans for Trajectories
	std::ifstream textFile(inputFile.c_str()); // open provided text file
	if (textFile.is_open()) { // if the file was able to successfully open
		while (std::getline(textFile,line)) { // while there is still a new line to take from
			// Create String Steamer and Get Command for Current Trajectory Point
			std::istringstream currentLine(line); // create a string steam element for delimiting data by spaces
			currentLine >> command; // get command name for current trajectory point

			// Get First Group Name and Joint Values for Current Trajectory Point
			currentLine >> groupName_1 >> totalJoints_1; //  get the command, group name for the first group, and total joints
			std::vector<double> jointValues_1(totalJoints_1); // create vector of doubles to store joint values for the first group
			for (int joint = 0; joint < totalJoints_1; joint++) { // iterate through joint values
				currentLine >> jointValues_1[joint]; // store the current joint value
			}

			// Get Second Group Name and Joint Values for Current Trajectory Point
			currentLine >> groupName_2 >> totalJoints_2; // get the group name for the second group and the total joints
			std::vector<double> jointValues_2(totalJoints_2); // create vector of doubles to store join values for the second group
			for (int joint = 0; joint < totalJoints_2; joint++) { // iterate through joint values
				currentLine >> jointValues_2[joint]; // store the current joint value
			}

			if (currentLine == 0) {
				if (groupName_index == 3) {
					if (((groupName_1.compare("right_arm") == 0) || (groupName_2.compare("right_arm") == 0)) && ((groupName_1.compare("left_arm") == 0) || (groupName_2.compare("left_arm") == 0)) {
						trajectory_joints.totalJoints = totalJoints_1 + totalJoints_2;
					} else {
						ROS_WARN("One of the group names within the provided input file is not recognized.");
						ROS_INFO("Recognized groups: left_arm, right_arm");
						ROS_INFO("Group names form file: %s, %s",groupName_1.c_str(),groupName_2.c_str());
						groupName_index = -1;
						break;
					}
				} else if (groupName_index == 2) {
					if (groupName_1.compare("right_arm") == 0) {
						trajectory_joints.totalJoints = totalJoints_1;
					} else if (groupName_2.compare("right_arm") == 0) {
						trajectory_joints.totalJoints = totalJoints_2;
					} else {
						ROS_WARN("One of the group names within the provided input file is not recognized.");
						ROS_INFO("Recognized groups: left_arm, right_arm");
						ROS_INFO("Group names form file: %s, %s",groupName_1.c_str(),groupName_2.c_str());
						groupName_index = -1;
						break;
					}
				} else if (groupName_index == 1) {
					if (groupName_1.compare("left_arm") == 0) {
						trajectory_joints.totalJoints = totalJoints_1;
					} else if (groupName_2.compare("left_arm") == 0) {
						trajectory_joints.totalJoints = totalJoints_2;
					} else {
						ROS_WARN("One of the group names within the provided input file is not recognized.");
						ROS_INFO("Recognized groups: left_arm, right_arm");
						ROS_INFO("Group names form file: %s, %s",groupName_1.c_str(),groupName_2.c_str());
						groupName_index = -1;
						break;
					}
				} else if (groupName_index == -1) {
					break;
				}
			}

			if (groupName_index == 1) {
				if (groupName_1.compare("left_arm") == 0) {
					trajectory_joints.joints[currentLine].joint.push_back(jointValues_1);
				} else if (groupName_2.compare("left_arm") == 0) {
					trajectory_joints.joints[currentLine].joint.push_back(jointValues_2);
				}
			} else if (groupName_index == 2) {
				if (groupName_1.compare("right_arm") == 0) {
					trajectory_joints.joints[currentLine].joint.push_back(jointValues_1);
				} else if (groupName_2.compare("right_arm") == 0) {
					trajectory_joints.joints[currentLine].joint.push_back(jointValues_2);
				}
			} else if (groupName_index == 3) {
				if (groupName_1.compare("left_arm") == 0) {
					for (int joint = 0; joint < trajectory_joints.totalJoints; joint++) {
						if (joint < totalJoints_1) { trajectory_joints.joints[currentLine].joint[joint] = jointValues_1[joint]; }
						else { trajectory_joints.joints[currentLine].joint[joint] = jointValues_2[joint-totalJoints_1]; }
					}
				} else
					for (int joint = 0; joint < trajectory_joints.totalJoints; joint++) {
						if (joint < totalJoints_2) { trajectory_joints.joints[currentLine].joint[joint] = jointValues_2[joint]; }
						else { trajectory_joints.joints[currentLine].joint[joint] = jointValues_1[joint-totalJoints_2]; }
					}
				}
			}

			currentLine++; // increment current line counter
		}
		textFile.close(); // close the text file
	} else { // if the file was not open successfully
		ROS_WARN("The provided file name could not be opened or does not exist."); // notify user of failure to open file
		ROS_INFO("File: %s",inputFile.c_str()); // notify user of the supplied file
	}

	if (groupName_index == -1) {
		trajectory_joints.totalJoints = 0;
	}

	return trajectory_joints;
}

trajectoryPoses getTrajectoryPoses(planningInterface::MoveGroup& group, std::string inputFile) {
	
	// Initialize Variables
	std::string line; // variable to store each line of text from the input file
	std::string command; // variable to store the command for each trajectory point from the input file
	int currentLine = 0;

	std::string groupName_1; // variable to store the first group name
	std::string groupName_2; // variable to store the second group name
	geometry_msgs::Pose pose_1;
	geometry_msgs::Pose pose_2;

	std::string groupName = group.getName();
	trajectoryPoses trajectory_poses;
	trajectory_poses.groupName = groupName;

	// Get Group Name
	int groupName_index;
	if (groupName.compare("left_arm") == 0) {
		groupName_index = 1;
	} else if (groupName.comapre("right_arm") == 0) {
		groupName_index = 2;
	} else if (groupName.comapre("both_arms") == 0) {
		groupName_index = 3;
	} else {
		groupName_index = -1;
		ROS_ERROR("Provided group name is not recognized.");
		ROS_INFO("Recognized group names list: left_arm, right_arm, both_arms");
		ROS_INFO("Provided group: %s",groupName.c_str());
	}

	// Notfify User that the Planner is About to Start
	ROS_INFO("Getting stored trajectory poses from file."); // notify user that the planner is about to start
	ROS_INFO("File location: %s",inputFile.c_str()); // notify user of the full path of the input file

	// Open File and Generate Plans for Trajectories
	std::ifstream textFile(inputFile.c_str()); // open provided text file
	if (textFile.is_open()) { // if the file was able to successfully open
		while (std::getline(textFile,line)) { // while there is still a new line to take from
			// Create String Steamer and Get Command for Current Trajectory Point
			std::istringstream currentLine(line); // create a string steam element for delimiting data by spaces
			currentLine >> command; // get command name for current trajectory point

			// Get First Group Names and Poses for Current Trajectory Point
			currentLine >> groupName_1; //  get the command and the group name for the first group
			currentLine >> pose_1.position.x >> pose_1.position.y >> pose_1.position.z; // get position for current pose from first group
			currentLine >> pose_1.orientation.x >> pose_1.orientation.y >> pose_1.orientation.z >> pose_1.orientation.w; // get orientation for current pose from first group

			currentLine >> groupName_2; //  get the group name for the second group
			currentLine >> pose_2.position.x >> pose_2.position.y >> pose_2.position.z; // get position for current pose from second group
			currentLine >> pose_2.orientation.x >> pose_2.orientation.y >> pose_2.orientation.z >> pose_2.orientation.w; // get orientation for current pose from second group

			// Check Group Names in Input File to Make Sure They Are Expected
			if (currentLine == 0) {
				if (groupName_index == 3) {
					if (!(((groupName_1.compare("right_arm") == 0) || (groupName_2.compare("right_arm") == 0)) && ((groupName_1.compare("left_arm") == 0) || (groupName_2.compare("left_arm") == 0))) {
						ROS_WARN("One of the group names within the provided input file is not recognized.");
						ROS_INFO("Recognized groups: left_arm, right_arm");
						ROS_INFO("Group names form file: %s, %s",groupName_1.c_str(),groupName_2.c_str());
						groupName_index = -1;
						break;
					}
				} else if (groupName_index == 2) {
					if ((groupName_1.compare("right_arm") != 0) && (groupName_2.compare("right_arm") != 0)) {
						ROS_WARN("One of the group names within the provided input file is not recognized.");
						ROS_INFO("Recognized groups: left_arm, right_arm");
						ROS_INFO("Group names form file: %s, %s",groupName_1.c_str(),groupName_2.c_str());
						groupName_index = -1;
						break;
					}
				} else if (groupName_index == 1) {
					if ((groupName_1.compare("left_arm") != 0) && (groupName_2.compare("left_arm") != 0)) {
						ROS_WARN("One of the group names within the provided input file is not recognized.");
						ROS_INFO("Recognized groups: left_arm, right_arm");
						ROS_INFO("Group names form file: %s, %s",groupName_1.c_str(),groupName_2.c_str());
						groupName_index = -1;
						break;
					}
				} else if (groupName_index == -1) {
					break;
				}
			}

			// Store the Current Pose into the Trajectory Structure
			if (groupName_index == 1) {
				if (groupName_1.compare("left_arm") == 0) {
					trajectory_poses.pose_left.push_back(pose_1);
				} else if (groupName_2.compare("left_arm") == 0) {
					trajectory_poses.pose_left.push_back(pose_2);
				}
			} else if (groupName_index == 2) {
				if (groupName_1.compare("right_arm") == 0) {
					trajectory_poses.pose_right.push_back(pose_1);
				} else if (groupName_2.compare("right_arm") == 0) {
					trajectory_poses.pose_right.push_back(pose_2);
				}
			} else if (groupName_index == 3) {
				if (groupName_1.compare("left_arm") == 0) {
					trajectory_poses.pose_left.push_back(pose_1);
					trajectory_poses.pose_right.push_back(pose_2);
				} else if (groupName_2.compare("left_arm") == 0) {
					trajectory_poses.pose_left.push_back(pose_2);
					trajectory_poses.pose_right.push_back(pose_1);
				}
			}

			currentLine++; // increment current line counter
		}
		textFile.close(); // close the text file
	} else { // if the file was not open successfully
		ROS_WARN("The provided file name could not be opened or does not exist."); // notify user of failure to open file
		ROS_INFO("File: %s",inputFile.c_str()); // notify user of the supplied file
	}

	return trajectory_poses;
}



planner generatePathsAndExecute(std::string inputFile, planningInterface::MoveGroup& group) {
/*  PROGRAMMER: Frederick Wachter - wachterfreddy@gmail.com
	DATE CREATE: 2016-06-22
	PURPOSE: Generate plans from input file for provided group

	INPUT(S):
		> inputFile - name of the file containing desured trajectory points
		> group - group to use for planning trajectories
	OUTPUT(S):
		< planner - planner structure, created to ensure proper execution is performed

	DEPENDENCIES: This function expects for two group joint values sets to be present in the provided file
	              The SRDF group for "both_arms" must first contain the joints for the "left_arm" then the joints for the "right_arm"
	INPUT FILE CONVENTION: The input file must have the following convention for each trajectory point:
		- $(Command Name) $(Group 1 Name) $(Group 1 Total Joints (N_1)) $(Joint 1 Value) ... $(Joint N_1 Value) $(Group 2 Name) $(Group 2 Total Joints (N_2)) $(Joint 1 Value) ... $(Joint N_2 Value)

	NOTE: Currently not doing anything with the joint command from provided input file | DATE: 2016-06-22
*/
	// Initialize Variables
	std::string line; // variable to store each line of text from the input file
	std::string command; // variable to store the command for each trajectory point from the input file

	std::string groupName_1; // variable to store the first group name
	std::string groupName_2; // variable to store the second group name
	std::string groupName_BothArms = "both_arms";
	std::vector<double>::size_type totalJoints_1; // variable to store the joint values for the first group
	std::vector<double>::size_type totalJoints_2; // variable to store the joint values for the second group

	planner plan; // structure of type planner
	plan.groupName = group.getName(); // get the group name of the provided group
	plan.success = false; // flag to indicate the success of the planner for provided trajectories

	planningInterface::MoveGroup::Plan currentPlan; // structure for an individual plan
	std::vector<planningInterface::MoveGroup::Plan> emptyPlan(1); // empty vector of plans with size of 1
	int planIndex = 0; // counter to indicate the current plan

	// Notfify User that the Planner is About to Start
	ROS_INFO("Opening file and starting planner."); // notify user that the planner is about to start
	ROS_INFO("File location: %s",inputFile.c_str()); // notify user of the full path of the input file

	// Open File and Generate Plans for Trajectories
	std::ifstream textFile(inputFile.c_str()); // open provided text file
	if (textFile.is_open()) { // if the file was able to successfully open
		while (std::getline(textFile,line)) { // while there is still a new line to take from
			// Create String Steamer and Get Command for Current Trajectory Point
			std::istringstream currentLine(line); // create a string steam element for delimiting data by spaces
			currentLine >> command; // get command name for current trajectory point

			// Get First Group Name and Joint Values for Current Trajectory Point
			currentLine >> groupName_1 >> totalJoints_1; //  get the command, group name for the first group, and total joints
			std::vector<double> jointValues_1(totalJoints_1); // create vector of doubles to store joint values for the first group
			for (int joint = 0; joint < totalJoints_1; joint++) { // iterate through joint values
				currentLine >> jointValues_1[joint]; // store the current joint value
			}

			// Get Second Group Name and Joint Values for Current Trajectory Point
			currentLine >> groupName_2 >> totalJoints_2; // get the group name for the second group and the total joints
			std::vector<double> jointValues_2(totalJoints_2); // create vector of doubles to store join values for the second group
			for (int joint = 0; joint < totalJoints_2; joint++) { // iterate through joint values
				currentLine >> jointValues_2[joint]; // store the current joint value
			}

			// Set Next Target for Planner
			if (plan.groupName.compare(groupName_1) == 0) { // if the desired move group is the first group from the current line
				group.setJointValueTarget(jointValues_1); // set the next target for the right arm
			} else if (plan.groupName.compare(groupName_2) == 0) { // if the desired move group is the second group from the current line
				group.setJointValueTarget(jointValues_2); // set the next target for the left arm
			} else if (plan.groupName.compare(groupName_BothArms) == 0) { // if the desired move group is both arms
				int totalJoints_DualArm = totalJoints_1 + totalJoints_2; // calculate total joints between both arms
				std::vector<double> jointValues_DualArm(totalJoints_DualArm); // create vector to store joint values for both arms
				for (int joint = 0; joint < totalJoints_DualArm; joint++) { // iterate through joint values for both arms
					if (groupName_1.compare("left_arm")) { // if the first group from the provided file gives joints values for left arm (left arm joints must be assigned first, as defined in the SRDF file)
						if (joint < totalJoints_1) { // if the current joint is less than the total joints in the left arm
							jointValues_DualArm[joint] = jointValues_1[joint]; // assign the current joint to the respective joint in the "both_arms" group
						} else { // if the current joint is supposed to be taken from the right arm joint values
							jointValues_DualArm[joint] = jointValues_2[joint-totalJoints_1]; // assign the current joint from the respective joint in the right arm
						}
					} else if (groupName_2.compare("left_arm")) { // if the second group from the provided file gives joint values for the right arm (right arm joints must be assigned second, as defined in the SRDF file)
						if (joint < totalJoints_2) { // if the current joint is less than the total joint in the left arm
							jointValues_DualArm[joint] = jointValues_2[joint]; // assign the current joint to the respective joint in the "both_arms" group
						} else { // if the current joint is supposed to be taken from the right arm joint values
							jointValues_DualArm[joint] = jointValues_1[joint-totalJoints_2]; // assign the current joint from the rspective joint in the right arm
						}
					}
				}
				group.setJointValueTarget(jointValues_DualArm); // set the next target for the "both_arms" group
			} else { // if the provided group name is not recognized as any of the group names above
				ROS_WARN("Provided group name is not recognized from group names supplied in file"); // notify user that the inputted group is not recognized
				ROS_INFO("Provided group name: %s",plan.groupName.c_str()); // notify user of the input group name
				ROS_INFO("Group 1: %s (from file)",groupName_1.c_str()); // notify user of the first group name pulled from the provided file
				ROS_INFO("Group 2: %s (from file)",groupName_2.c_str()); // notfiy user of the second group name pulled from the provided file
				ROS_INFO("Other Possible Group(s): %s (from SRDF)",groupName_BothArms.c_str());
				plan.success = false; // set the planner success flag as failed
				break; // break from the while loop
			}

			// Plan Trajectory
			currentPlan = emptyPlan[0]; // reset the current plan variable
			group.setStartStateToCurrentState(); // set the start location to the current location of the robot
			plan.success = group.plan(currentPlan); // create a path plan for current trajectory point
			planIndex++; // increment plan index

			// Add Planned Trajectory to Planner Vector if Plan Was Successful
			if (plan.success == true) { // if the planning was successful
				plan.plans.push_back(currentPlan); // add the newly create plan to the plan vector
				ROS_INFO("Plan create for trajectory point: %d",planIndex); // notify user of the trajectory point which planning was executed for
			} else { // if the planning was not successful
				ROS_WARN("Plan failed for trajectory point: %d",planIndex); // notify user of the trajectory point which planning failed to execute for
				break; // break from the while loop
			}

			// Execute Newly Create Plan
			ROS_INFO("Executing trajectory."); // notify user that the execution of the new plan is about to start
			plan.success = group.execute(currentPlan); // execute trajectory for the current plan
			if (executeSuccess_flag == true) { // if the execution was successful
				ROS_INFO("Trajectory %d executed",planIndex+1); // notify user of the plan number that was executed successfully
			} else { // if the execution was not successful
				ROS_WARN("Failed to execute trajectory for plan: %d",planIndex+1); // notify user of the plan number that was not executed successfully
				break; // break from the for loop
			}
		}
		textFile.close(); // close the text file
	} else { // if the file was not open successfully
		ROS_WARN("The provided file name could not be opened or does not exist."); // notify user of failure to open file
		ROS_INFO("File: %s",inputFile.c_str()); // notify user of the supplied file
	}

	// Notify User if the Planner Was Successful
	if (plan.success == true) { // if the planner was successful
		ROS_INFO("Finished planning."); // notify user that the planner has finished
		return plan; // return the planner success flag and the vector of plans
	} else { // if the planner was not successful
		ROS_ERROR("Planning or executing error occurred, exiting planner function."); // notify user that the planner failed
		return plan; // return the planner success flag and the empty vector of plans
	}
}

planner generatePaths(std::string inputFile, planningInterface::MoveGroup& group) {
/*  PROGRAMMER: Frederick Wachter - wachterfreddy@gmail.com
	DATE CREATE: 2016-06-22
	PURPOSE: Generate plans from input file for provided group

	INPUT(S):
		> inputFile - name of the file containing desured trajectory points
		> group - group to use for planning trajectories
	OUTPUT(S):
		< planner - planner structure, created to ensure proper execution is performed

	DEPENDENCIES: This function expects for two group joint values sets to be present in the provided file
	              The SRDF group for "both_arms" must first contain the joints for the "left_arm" then the joints for the "right_arm"
	INPUT FILE CONVENTION: The input file must have the following convention for each trajectory point:
		- $(Command Name) $(Group 1 Name) $(Group 1 Total Joints (N_1)) $(Joint 1 Value) ... $(Joint N_1 Value) $(Group 2 Name) $(Group 2 Total Joints (N_2)) $(Joint 1 Value) ... $(Joint N_2 Value)

	NOTE: Currently not doing anything with the joint command from provided input file | DATE: 2016-06-22
*/
	// Initialize Variables
	std::string line; // variable to store each line of text from the input file
	std::string command; // variable to store the command for each trajectory point from the input file

	std::string groupName_1; // variable to store the first group name
	std::string groupName_2; // variable to store the second group name
	std::string groupName_BothArms = "both_arms";
	std::vector<double>::size_type totalJoints_1; // variable to store the joint values for the first group
	std::vector<double>::size_type totalJoints_2; // variable to store the joint values for the second group

	planner plan; // structure of type planner
	plan.groupName = group.getName(); // get the group name of the provided group
	plan.success = false; // flag to indicate the success of the planner for provided trajectories

	planningInterface::MoveGroup::Plan currentPlan; // structure for an individual plan
	std::vector<planningInterface::MoveGroup::Plan> emptyPlan(1); // empty vector of plans with size of 1
	int planIndex = 0; // counter to indicate the current plan

	// Notfify User that the Planner is About to Start
	ROS_INFO("Opening file and starting planner."); // notify user that the planner is about to start
	ROS_INFO("File location: %s",inputFile.c_str()); // notify user of the full path of the input file

	// Open File and Generate Plans for Trajectories
	std::ifstream textFile(inputFile.c_str()); // open provided text file
	if (textFile.is_open()) { // if the file was able to successfully open
		while (std::getline(textFile,line)) { // while there is still a new line to take from
			// Set Start State Planner Parameter for Provided Group
			if (planIndex == 0) { // if first time running through the while loop
				group.setStartStateToCurrentState(); // set the start location to the current location of the robot
			} else { // if it is not the first time running through the while loop
				moveitCore::RobotState state(group.getRobotModel()); // create state structure using the robots current state
				moveitCore::jointTrajPointToRobotState(currentPlan.trajectory_.joint_trajectory, (currentPlan.trajectory_.joint_trajectory.points.size()-1), state); // update the state structure to the given trajectory point
				group.setStartState(state); // set the start state as the newly generated state
			}

			// Create String Steamer and Get Command for Current Trajectory Point
			std::istringstream currentLine(line); // create a string steam element for delimiting data by spaces
			currentLine >> command; // get command name for current trajectory point

			// Get First Group Name and Joint Values for Current Trajectory Point
			currentLine >> groupName_1 >> totalJoints_1; //  get the command, group name for the first group, and total joints
			std::vector<double> jointValues_1(totalJoints_1); // create vector of doubles to store joint values for the first group
			for (int joint = 0; joint < totalJoints_1; joint++) { // iterate through joint values
				currentLine >> jointValues_1[joint]; // store the current joint value
			}

			// Get Second Group Name and Joint Values for Current Trajectory Point
			currentLine >> groupName_2 >> totalJoints_2; // get the group name for the second group and the total joints
			std::vector<double> jointValues_2(totalJoints_2); // create vector of doubles to store join values for the second group
			for (int joint = 0; joint < totalJoints_2; joint++) { // iterate through joint values
				currentLine >> jointValues_2[joint]; // store the current joint value
			}

			// Set Next Target for Planner
			if (plan.groupName.compare(groupName_1) == 0) { // if the desired move group is the first group from the current line
				group.setJointValueTarget(jointValues_1); // set the next target for the right arm
			} else if (plan.groupName.compare(groupName_2) == 0) { // if the desired move group is the second group from the current line
				group.setJointValueTarget(jointValues_2); // set the next target for the left arm
			} else if (plan.groupName.compare(groupName_BothArms) == 0) { // if the desired move group is both arms
				int totalJoints_DualArm = totalJoints_1 + totalJoints_2; // calculate total joints between both arms
				std::vector<double> jointValues_DualArm(totalJoints_DualArm); // create vector to store joint values for both arms
				for (int joint = 0; joint < totalJoints_DualArm; joint++) { // iterate through joint values for both arms
					if (groupName_1.compare("left_arm")) { // if the first group from the provided file gives joints values for left arm (left arm joints must be assigned first, as defined in the SRDF file)
						if (joint < totalJoints_1) { // if the current joint is less than the total joints in the left arm
							jointValues_DualArm[joint] = jointValues_1[joint]; // assign the current joint to the respective joint in the "both_arms" group
						} else { // if the current joint is supposed to be taken from the right arm joint values
							jointValues_DualArm[joint] = jointValues_2[joint-totalJoints_1]; // assign the current joint from the respective joint in the right arm
						}
					} else if (groupName_2.compare("left_arm")) { // if the second group from the provided file gives joint values for the right arm (right arm joints must be assigned second, as defined in the SRDF file)
						if (joint < totalJoints_2) { // if the current joint is less than the total joint in the left arm
							jointValues_DualArm[joint] = jointValues_2[joint]; // assign the current joint to the respective joint in the "both_arms" group
						} else { // if the current joint is supposed to be taken from the right arm joint values
							jointValues_DualArm[joint] = jointValues_1[joint-totalJoints_2]; // assign the current joint from the rspective joint in the right arm
						}
					}
				}
				group.setJointValueTarget(jointValues_DualArm); // set the next target for the "both_arms" group
			} else { // if the provided group name is not recognized as any of the group names above
				ROS_WARN("Provided group name is not recognized from group names supplied in file"); // notify user that the inputted group is not recognized
				ROS_INFO("Provided group name: %s",plan.groupName.c_str()); // notify user of the input group name
				ROS_INFO("Group 1: %s (from file)",groupName_1.c_str()); // notify user of the first group name pulled from the provided file
				ROS_INFO("Group 2: %s (from file)",groupName_2.c_str()); // notfiy user of the second group name pulled from the provided file
				ROS_INFO("Other Possible Group(s): %s (from SRDF)",groupName_BothArms.c_str());
				plan.success = false; // set the planner success flag as failed
				break; // break from the while loop
			}

			// Plan Trajectory
			currentPlan = emptyPlan[0]; // reset the current plan variable
			plan.success = group.plan(currentPlan); // create a path plan for current trajectory point
			planIndex++; // increment plan index

			// Add Planned Trajectory to Planner Vector if Plan Was Successful
			if (plan.success == true) { // if the planning was successful
				plan.plans.push_back(currentPlan); // add the newly create plan to the plan vector
				ROS_INFO("Plan create for trajectory point: %d",planIndex); // notify user of the trajectory point which planning was executed for
			} else { // if the planning was not successful
				ROS_WARN("Plan failed for trajectory point: %d",planIndex); // notify user of the trajectory point which planning failed to execute for
				break; // break from the while loop
			}
		}
		textFile.close(); // close the text file
	} else { // if the file was not open successfully
		ROS_WARN("The provided file name could not be opened or does not exist."); // notify user of failure to open file
		ROS_INFO("File: %s",inputFile.c_str()); // notify user of the supplied file
	}

	// Notify User if the Planner Was Successful
	if (plan.success == true) { // if the planner was successful
		ROS_INFO("Finished planning."); // notify user that the planner has finished
		return plan; // return the planner success flag and the vector of plans
	} else { // if the planner was not successful
		ROS_ERROR("Planning error occurred, exiting planner function."); // notify user that the planner failed
		return plan; // return the planner success flag and the empty vector of plans
	}

}

bool executePaths(planningInterface::MoveGroup& group, planner& plan) {
/*  PROGRAMMER: Frederick Wachter - wachterfreddy@gmail.com
	DATE CREATE: 2016-06-22
	PURPOSE: Execute provided plans for provided group

	INPUT(S):
		> group - group to use to execute planned trajectories
		> plans - vector of plans that have been previously generated
	OUTPUT(S):
		< exectuteSuccess_flag - bool to indicate if the execution of all plans was successful or not successful

	DEPENDENCIES: The generatePaths() function must be run and return succesful before execution this function
*/
	// Initialize Variables
	bool executeSuccess_flag;// flag to indicate the success of the execution for provided plans

	// Ensure Proper Group is Used for Provided Planner
	if (plan.groupName.compare(group.getName()) != 0) { // if the given group does not have the same name as the name within the planner object
		ROS_WARN("Provided planner is not meant for the provided group"); // notify the user that the provided planner structure is not for the provided group
		ROS_INFO("Planner Group: %s",plan.groupName.c_str()); // notify user of the group the planner is intended to be used for
		ROS_INFO("Provided Group: %s",group.getName().c_str()); // notify user of the name of the supplied group
		executeSuccess_flag = false; // set success flag to indicate the execution was not successful
	} else if (plan.success == 0) {
		ROS_WARN("Provided planner did not successfully create plans for one or all trajectory points for supplid text file.");
		ROS_INFO("Please (re-)run the generatePaths() function before executing paths again");
		executeSuccess_flag = false; // set success flag to indicate the execution was not successful
	} else {
		// Notify User that the Plan Exection is About to be Started
		ROS_INFO("Executing trajectories."); // notify user that the execution of the plans is about to start

		// Execute Given Plans
		for (int currentPlan = 0; currentPlan < plan.plans.size(); currentPlan++) { // for all plans in plans vector
			executeSuccess_flag = group.execute(plan.plans[currentPlan]); // execute trajectory for the current plan
			if (executeSuccess_flag == true) { // if the execution was successful
				ROS_INFO("Trajectory %d executed",currentPlan+1); // notify user of the plan number that was executed successfully
			} else { // if the execution was not successful
				ROS_WARN("Failed to execute trajectory for plan: %d",currentPlan+1); // notify user of the plan number that was not executed successfully
				break; // break from the for loop
			}
		}
	}

	// Notify User if the Execition Was Successful
	if (executeSuccess_flag == true) { // if the execution was successful
		ROS_INFO("Finished executing trajectories."); // notify the user that the execution has finished
	} else { // if the execution was not successful
		ROS_ERROR("Execution error occurred, exiting execution function."); // notify the user that the execution failed
	}

	return executeSuccess_flag; // return the execution success flag
}


