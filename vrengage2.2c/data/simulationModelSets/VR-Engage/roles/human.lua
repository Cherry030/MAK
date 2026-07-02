inherits = "@(vre-roles-dir)/humanBase.lua";
displayLayouts = {
   "@(vre-roles-dir)/displayLayouts/human1ScreenHorizontal.lua";
   "@(vre-roles-dir)/displayLayouts/humanVrLayouts.lua";
};


-- Add new notify logic component.
components={ 
   --[[
   --FE ��ũ�� �÷�����
   ["nesslabObserver"] = {
      componentType = "DtObserver";
   };




   --FE �ൿ �÷�����
   ["nesslabAction"] = {
      componentType = "DtAction";
   };

   
   --FE �����ѱ�+�ѱ���� �÷�����
   ["nesslabWeapon"] = {
      componentType = "DtWeapon";
   };



   --humanBase.lua�� �ִ� humanWeaponPoseLogic ComponentType�� ����
   --FE �ѱ����(����������), humanWeaponPoseLogic �̸� �����ϸ� ���۾���(����x)
   ["humanWeaponPoseLogic"] = {
      componentType = "DtNesslabWeaponPoseLogic";
      priority = 5;
   };




   --humanBase.lua�� �ִ� DtHumanControlLogic ComponentType�� ����
   --this replaces the "controlLogic" from the baseRole.lua
   ["controlLogic"] = { 
      componentType = "DtCustomHumanControlLogic";
      priority = 5;
      sprintMultiplier = 1.555556; 
      sprintBackwardMultiplier = 1.2;
      --sprintMultiplier = 1; 
      --sprintBackwardMultiplier = 1;
      
      inputConfigFile = "humanInput.lua";
      joystickFunctionGroups={
         "Human Movement",
         "Human Combat",
         "Observer Control",
         "Parachute",
         "Radio"
      };
      mouseControlSupport = true;
   
      --Do we want view toggling to be available?
      isAllowedThirdPersonView = true;

      moveDelays = {
         toProne = 0.5,
         fromProne = 1.0,
         fromSitting = 1.0,
         fromKneeling = 2.0
      };
      
      maxViewMagnification=7;
      raiseGunOnAttach = true;
      aimingErrorCalcMaxSpread = 96;  -- in inches - aiming error calculation - the max spread of each shot
      aimingErrorCalcDistanceToTarget = 50; -- in meters - aiming error calculation - the spread occurs at this distance
      requiredAimingErrorPercent = 0.07;  -- a percentage - always want a little error in the aiming so all shots don't hit the same spot
      proneAimingImprovementPercent = 0.2;  -- a percentage - the amount of percent aiming improves when in the prone position
      aimingImprovementPercent = 0.2;  -- a percentage - the amount of percent aiming improves when aiming the weapon (using the weapon aiming capability)
      showCompass = true;
      -- compassUnits = "NATO-MIL"; -- "Degrees" or "NATO-MIL", defaults to "Degrees"
      binocularsInputMultiplier = 0.1; -- scales the pitch and yaw input values while using binoculars
   };
   ]]--
   
   --[[
   ["dstNetwork"] = {
      componentType = "DtDstNetwork";

            extendedState = {
         {
            name = "treadmill";
            type = "int";
         };
         {
            name = "manipulator";
            type = "int";
         };
         {
            name = "helmet-patch";
            type = "int";
         };
         {
            name = "body-patch";
            type = "int";
         };
         {
            name = "hand-patch";
            type = "int";
         };
      };
   }; 
   ]]--
      --DeviceState
   ["deviceStatePlugin"] = {
      componentType = "DtDevState";

            extendedState = {
         {
            name = "treadmill-type";
            type = "int";
         };
         {
            name = "treadmill";
            type = "int";
         };
         {
            name = "manipulator";
            type = "int";
         };
         {
            name = "helmet-patch";
            type = "int";
         };
         {
            name = "body-patch";
            type = "int";
         };
         {
            name = "hand-patch";
            type = "int";
         };
      };
   };

   --TITS
   ["titsPlugin"] = {
      componentType = "DtTITS";

      extendedState = {
         {
            name = "helmet-patch";
            type = "int";
         };
         {
            name = "body-patch";
            type = "int";
         };
         {
            name = "hand-patch";
            type = "int";
         };
      };
   };

   --[[
   --RFID
   ["rfidPlugin"] = {
      componentType = "DtRFID";
   };

   

   --DeviceState
   ["deviceStatePlugin"] = {
      componentType = "DtDevState";
   };
   ]]--
};

