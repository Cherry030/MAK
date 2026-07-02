--- Role configuration for humanBase.lua
appModes = {"Ground"};
roles={"Human"};
inherits = "@(vre-roles-dir)/baseRole.lua";

menuConfig = {
   menus = {
      ["Action"] = {
         parent=""; --parent isn't needed if it's a top level menu
         size = {
            x = 300;
            y = 300;
         }; -- this is inherited if its a child menu
         position = {
            x = 100;
            y = 300;
         }; -- this is inherited if its a child menu
         elements={
            {
               label = "Stand Up";
               action = "standUp";
               displayConditionList = {
                  "sitting";
                  "hostWalkable";
               };
            };
            {
               label = "Exit Launcher";
               action = "exitLauncher";
               displayCondition = "usingLauncher";
            };
            {
               label = "Weapon State: $(weaponState)";
               action = "cycleWeaponState";
               valueMapping = {
                  "Stowed";
                  "Stowed";
                  "Ready";
                  "Aiming";
               };
               displayCondition = "hasWeapons";
            };
            {
               label = "Weapon: $(currentWeapon)";
               action = "selectMenu";
               param = "Weapons";
               displayCondition = "hasWeapons";
            };
            {
               label = "Reload";
               action = "reload";
               displayConditionList = {
                  "weaponDeployed";
                  "weaponHasClip";
               };
            };
            {
               label = "Drop Injured";
               action = "dropInjured";
               displayCondition = "hasEmbarkedEntities";
            };
            {
               label = "Night Vision: $(NVG)";
               action = "toggleNvg";
               valueMapping = {"OFF"; "ON"};
               displayConditionList = {"!isCivilian"};
            };
            {
               label = "Flashlight: $(flashlight)";
               action = "toggleFlashlight";
               valueMapping = {"OFF"; "ON"};
               displayCondition = "!isCivilian";
            };
            {
               label = "Compass: $(compass)";
               action = "toggleCompass";
               valueMapping = {"OFF"; "ON"};
               displayCondition = "!vr-enabled";
            };
            {
               label = "Compass Units: $(compassUnits)";
               action = "toggleCompassUnits";
               valueMapping = {"Degrees"; "NATO-MIL"};
               displayCondition = "!vr-enabled";
            };
            {
               label = "North Reference: $(northReference)";
               action = "toggleNorthReference";
               displayCondition = "!vr-enabled";
            };
            {
               label = "Task Entity";
               action = "selectMenu";
               param = "Select Entity";
               displayCondition = "taskableEntities";
            };
            {
               label = "Gesture";
               action = "selectMenu";
               param = "Gesture";
            };
            -- {label = "Execute Script", action = "selectMenu", param = "Scripts"};
            {
               label = "View Mode: $(observerAttachMode)";
               action = "toggleView";
               valueMapping = {"1st Person"; "3rd Person"};
               displayConditionList = {
                  "isAllowedThirdPersonView";
                  "!vr-enabled";
                  "!force1stPerson";
                  "!force3rdPerson";
         };
      };
            {
               label = "Load Resource";
               action = "selectMenu";
               param = "Load Resource";
               displayCondition = "admin";
   };
};
      };
   };
};

components={   
   -- Component for managing extended state attributes beyond the standard set
   ["extendedStateLogic"] = {
      componentType = "DtExtendedStateLogic";
      priority = 4;

      -- Table containing definitions for extended state attributes with name and type fields.
      -- OPTIONAL, Type: table, Default: {}
      extendedState={
         {
            name = "attackMode";
            type = "string";
      };
      };

      -- Table containing definitions for state properties with name and type fields.
      -- OPTIONAL, Type: table, Default: {}
      stateProperties={
         {
            name = "OverallHealth";
            type = "int";
      };
         {
            name = "MobilityHealth";
            type = "int";
   };
         {
            name = "FirepowerHealth";
            type = "int";
         };
         {
            name = "Fatigue";
            type = "float";
         };
         {
            name = "Oxygen";
            type = "float";
         };
         {
            name = "PercentOfMaxSpeed";
            type = "float";
         };
         {
            name = "AmmoInClips";
            type = "ammoClipMap";
         };
         {
            name = "ReloadTimer";
            type = "float";
         };
         {
            name = "HasLaser";
            type = "bool";
         };
         {
            name = "laserCode";
            type = "int";
         };
      };
   };
   
   --replaces one in base class
   -- DtActionMenuLogic component for managing logic operations
   ["actionMenuLogic"] = {

      -- Distance threshold for handling overlapping slot proximity zones.
      -- OPTIONAL, Type: double, Default: "varies"
      -- slotProximityOverlapDist = "varies";
      componentType = "DtActionMenuLogic";
      priority = 5;
      
      -- Configuration table defining quick menus for different entity types and interaction contexts.
      -- OPTIONAL, Type: table, Default: {}
      entityQuickMenus = {
         -- These map to menus defined in @(vre-roles-dir)/menus/quickMenus.lua
         ["5:1:0:5:50:1:0"] = "Door";
         ["5:1:0:5:50:2:0"] = "Window";
         ["5:1:225:2:1:1:0"] = "Security Station";
         ["5:1:225:9:1:1:1"] = "Javelin Launcher";
         ["9:1:225:1:1:0:0"] = "LLDR";
         ["1:1:225:7:5:0:0"] = "Truck"; --5 ton truck

         ["3:-1:-1:-1:-1:-1:-1"] = "Person";
         
         -- Default catch-all wildcard mapping for all Platforms:
         ["1:-1:-1:-1:-1:-1:-1"] = "Vehicle";
         
         -- Generic embarkation menu for vehicle w/o slots:
         -- Uncomment this menu and set the entity type to
         -- allow walk-up embarkation on an unconfigured
         -- vehicle that does not have defined slots.
         -- Human will be embarked invisible and player
         -- will be put in spectator mode watching vehicle.
         -- Duplicate this line to map as many such entity
         -- types as required. Alternatively, uncomment
         -- this line and leave wildcard mapping, and
         -- comment Default catch-all wildcard mapping
         -- above if it is desired that a non-slot-enabled
         -- embarkation be the default. Explicit type
         -- mappings will then be required for all
         -- specific vehicles where slot embarkation is
         -- desired.
         --["1:-1:-1:-1:-1:-1:-1"] = "Embark Notional";
      };
      
      -- Maximum distance in meters for slot proximity detection.
      -- OPTIONAL, Type: double, Default: "varies"
      slotProximityMaxDistance = 3.0;

      -- Z-axis scaling factor for slot proximity calculations.
      -- OPTIONAL, Type: double, Default: "varies"
      slotProximityZScale = 0.4;

      -- Weight factor for angle consideration in slot proximity calculations.
      -- OPTIONAL, Type: double, Default: "varies"
      slotProximityAngleWeight = 2.0;

      -- Field of view angle in degrees for slot proximity detection.
      -- OPTIONAL, Type: double, Default: "varies"
      slotProximityFov = 120;

      -- Maximum distance for rear-facing slot proximity detection.
      -- OPTIONAL, Type: double, Default: "varies"
      slotProximityRearMaxDist = 1.0;
      alwaysShowRoleSlots = true;
   };
   
   -- Component for managing weapon resources, equipment, and related state attributes
   ["weaponResourceLogic"] = {
      componentType = "DtWeaponResourceLogic";
      priority = 5;

      -- Configuration table mapping resource names to display names and properties.
      -- OPTIONAL, Type: table, Default: {}
      resourceMappings = {
         -- Table key is the Resource that determines if this weapon is available
         ["2:8:225:2:1:1:0"] = { -- ["M16A2-556mm"] 
            -- munitionType is munition type to fire; if undefined, key resource is used.
            munitionType = "2:8:225:2:1:1:0"; -- M16A2-556mm 
            -- shortName is the weapon name displayed in the Action Menu
            shortName = "M4";
            -- diguyHandItem specifies a DIGuy hand item to be used when this weapon is selected
            diguyHandItem = "M4A1Carbine";
            -- itemMatches is a list of DIGuy hand items which map to this weapon. This list is used
            -- to determine the initial weapon selection when the player first engages a soldier.
            itemMatches = {
               "m240";
               "M4A1Carbine";
               "m249";
               "m4_m203";
               "m16_m203";
               "m60";
               "m16";
               "sar_21";
               "iwi_x95";
            };
            -- gunZoomLevel is the default zoom level when aiming down sights with this weapon
            gunZoomLevel = 2.2;
            -- type determines the UI/UX behavior of this weapon
            type = "gun";
            -- icon is the path to the image file that will be shown in the weapon list in the HUD
            icon = "$(SHARED_DATA_DIR)/Overlays/assault_rifle_icon.png";
            -- showEmpty flag determines whether the weapon should be displayed if the key resource count is 0, default false
            showEmpty = true;
            -- order specifies the order in which the weapons appear in the HUD and Action Menu
            order = 1;
         };
         ["6:2:225:7:50:60:0"] = { -- Javelin
            munitionType = "2:2:225:1:8:0:0";
            rangeId = "FGM-148 Javelin";
            shortName = "Javelin";
            diguyHandItem = "javelin";
            type = "launcher";
            taskType = "fireOnLocation";
            icon = "$(SHARED_DATA_DIR)/Overlays/javelin_icon.png";
            qmlOverlayComponent = "javelinOverlay";
            showGrenadeReticle = false;
            blockBinoculars = true;
            zoomLevels = {4; 9};
            sensorMode = "IR";
            twoStageFire = true;
            --secondStageZoomLevels = { 9 },
            requiresTargetLock = true;
            aimMissileInFlight = false;
            showEmpty = false;
            deployedEquipmentTypes = {"5:1:225:9:1:1:1"};
            order = 8;
         };
         ["6:2:222:7:45:110:0"] = { -- RPG
            munitionType = "2:2:222:2:8:1:0";
            rangeId = "Handheld RPG Launcher";
            shortName = "RPG";
            diguyHandItem = "rpg";
            type = "launcher";
            taskType = "fireOnLocation";
            icon = "$(SHARED_DATA_DIR)/Overlays/RPG_icon.png";
            qmlOverlayComponent = "rpgOverlay";
            showGrenadeReticle = false;
            blockBinoculars = true;
            zoomLevels = {2};
            twoStageFire = false;
            showEmpty = false;
            order = 8;
         };
         ["2:8:225:2:1:1:1"] = { -- ["EF88-556mm"]
            shortName = "EF88";
            diguyHandItem = "EF88AusteyrAssaultRifle";
            itemMatches={"EF88AusteyrAssaultRifle"};
            type = "gun";
            gunZoomLevel = 2.2;
            icon = "$(SHARED_DATA_DIR)\\Overlays\\assault_rifle_icon.png";
            showEmpty = true;
            order = 1;
         };
         ["2:9:225:2:4:2:0"] = { -- ["M433-40mm-HEDP-grenade-cartridge"]
            shortName = "40mm HE";
            diguyHandItem = "ef88";
            itemMatches = {"ef88"};
            type = "launcher";
            taskType = "fireForEffect";
            icon = "$(SHARED_DATA_DIR)\\Overlays\\40mmGrenadeHE.png";
            order = 2;
         };
         ["2:9:225:2:4:12:0"] = { -- ["M583A1Illum"]
            shortName = "M583A 1Illum";
            diguyHandItem = "ef88";
            itemMatches={"ef88"};
            type = "launcher";
            taskType = "fireForEffect";
            icon = "$(SHARED_DATA_DIR)\\Overlays\\40mmGrenadeHE.png";
            order = 2;
         };
         ["2:9:225:2:4:14:0"] = { -- ["M651TearGas"]
            shortName = "M651 Tear Gas";
            diguyHandItem = "ef88";
            itemMatches={"ef88"};
            type = "launcher";
            taskType = "fireForEffect";
            icon = "$(SHARED_DATA_DIR)\\Overlays\\40mmGrenadeSG.png";
            order = 2;
         };
         ["2:9:225:2:4:20:0"] = { -- ["M713Red"]
            shortName = "40mm SG";
            diguyHandItem = "ef88";
            itemMatches={"ef88"};
            type = "launcher";
            taskType = "fireForEffect";
            icon = "$(SHARED_DATA_DIR)\\Overlays\\40mmGrenadeSG.png";
            order = 3;
         };
         ["2:9:225:2:4:21:0"] = { -- ["M715Green"]
            shortName = "M715 Green";
            diguyHandItem = "ef88";
            itemMatches = {"ef88"};
            type = "launcher";
            taskType = "fireForEffect";
            icon = "$(SHARED_DATA_DIR)\\Overlays\\40mmGrenadeSG.png";
            order = 3;
         };
         ["2:9:225:2:4:22:0"] = { -- ["M716Yellow"]
            shortName = "M716 Yellow";
            diguyHandItem = "ef88";
            itemMatches = {"ef88"};
            type = "launcher";
            taskType = "fireForEffect";
            icon = "$(SHARED_DATA_DIR)\\Overlays\\40mmGrenadeSG.png";
            order = 3;
         };
         ["2:2:225:2:1:1:1"] = { -- ["XM107-50cal"]
            shortName = "XM107";
            diguyHandItem = "XM107SniperRifle";
            itemMatches = {"XM107SniperRifle"};
            type = "gun";
            gunZoomLevel = 2.2;
            hideFirstPersonModelWhenAiming = true;
            qmlOverlayComponent = "xm107Overlay";
            icon = "$(SHARED_DATA_DIR)\\Overlays\\assault_rifle_icon.png";
            order = 1;
         };
         ["2:8:222:2:2:3:0"] = { -- AKA-762mm
            shortName = "AK-47";
            diguyHandItem = "ak47";
            type = "gun";
            gunZoomLevel = 2.2;
            icon = "$(SHARED_DATA_DIR)/Overlays/ak47_icon.png";
            showEmpty = true;
            order = 2;
         };
         ["2:8:225:2:3:0:0"] = { -- ["M9-9mm"]
            shortName = "M9";
            diguyHandItem = "m9";
            type = "gun";
            gunZoomLevel = 1.2;
            icon = "$(SHARED_DATA_DIR)/Overlays/m9_icon.png";
            showEmpty = true;
            order = 4;
         };
         ["2:9:225:2:85:1:0"] = { -- M67
            shortName = "Frag Grenade";
            diguyHandItem = "grenade";
            type = "grenade";
            icon = "$(SHARED_DATA_DIR)/Overlays/fragGrenade.png";
            order = 5;
         };
         ["2:9:225:2:85:3:0"] = { -- M18-green
            shortName = "Smoke Grenade";
            diguyHandItem = "grenade";
            type = "grenade";
            icon = "$(SHARED_DATA_DIR)/Overlays/smoke.png";
            order = 6;
         };
         ["2:9:225:2:85:7:0"] = { -- M84
            shortName = "Flash Bang";
            diguyHandItem = "grenade";
            type = "grenade";
            icon = "$(SHARED_DATA_DIR)/Overlays/flashBang.png";
            order = 7;
         };

         ["2:8:225:2:1:1:2"] = { -- ["K2-556mm"]
            shortName = "K2";
            diguyHandItem = "rifle_k2";
            itemMatches={"rifle_k2"};
            type = "gun";
            gunZoomLevel = 2.2;
            icon = "$(SHARED_DATA_DIR)\\Overlays\\assault_rifle_icon.png";
            showEmpty = true;
            order = 1;
         };
         ["2:8:225:2:3:0:1"] = { -- ["K5-9mm"]
            shortName = "K5";
            diguyHandItem = "pistol_k5";
            type = "gun";
            gunZoomLevel = 1.2;
            icon = "$(SHARED_DATA_DIR)/Overlays/m9_icon.png";
            showEmpty = true;
            order = 4;
         };
      };
   };
   
   --this replaces the "controlLogic" from the baseRole.lua
   -- Control logic component for the human entity
   ["controlLogic"] = {

      -- Movement speed multiplier when aiming weapons.
      -- OPTIONAL, Type: double, Default: 0.46666667
      -- aimMultiplier = 0.46666667;

      -- Whether to invert pitch input controls.
      -- OPTIONAL, Type: bool, Default: false
      -- invertPitchInput = false;

      -- Sensitivity multiplier for joystick input.
      -- OPTIONAL, Type: double, Default: 3.0
      -- joystickSensitivity = 3.0;

      -- Table of postures that prevent movement.
      -- OPTIONAL, Type: table, Default: {}
      -- moveLockedPostures = {};

      -- Table defining the sequence of available postures.
      -- OPTIONAL, Type: table, Default: {}
      -- postureSequence = {};

      -- Movement speed multiplier when running.
      -- OPTIONAL, Type: double, Default: 0.77777778
      -- runMultiplier = 0.77777778;

      -- Movement speed multiplier when walking.
      -- OPTIONAL, Type: double, Default: 0.3
      -- walkMultiplier = 0.3;

      -- Table of postures that prevent weapon usage.
      -- OPTIONAL, Type: table, Default: {}
      -- weaponLockedPostures = {};

      -- Table of postures that prevent weapon usage when moving.
      -- OPTIONAL, Type: table, Default: {}
      -- weaponLockedWhenMovingPostures = {};
      componentType = "DtHumanControlLogic";
      priority = 5;

      -- Movement speed multiplier when sprinting forward.
      -- OPTIONAL, Type: double, Default: 1.555556
      sprintMultiplier = 1.555556; 

      -- Movement speed multiplier when sprinting backward.
      -- OPTIONAL, Type: double, Default: 1.2
      sprintBackwardMultiplier = 1.2;
      
      inputConfigFile = "humanInput.lua";
      joystickFunctionGroups={
         "Human Movement";
         "Human Combat";
         "Observer Control";
         "Parachute";
         "Radio";
      };
      mouseControlSupport = true;
   
      --Do we want view toggling to be available?

      -- Whether third-person view mode is allowed.
      -- OPTIONAL, Type: bool, Default: false
      isAllowedThirdPersonView = true;

      -- Table of posture change animation delays and timing.
      -- OPTIONAL, Type: table, Default: {}
      moveDelays = {
         toProne = 0.5;
         fromProne = 1.0;
         fromSitting = 1.0;
         fromKneeling = 2.0;
      };
      
      maxViewMagnification=7;

      -- Whether to automatically raise weapon when attaching to entity.
      -- OPTIONAL, Type: bool, Default: false
      raiseGunOnAttach = true;

      -- Random error applied to player aim

      -- Maximum aiming spread in inches for error calculations.
      -- OPTIONAL, Type: double, Default: 0.0
      aimingErrorCalcMaxSpread = 96;  -- the max random spread of each shot, in inches, at the reference distance

      -- Target distance in meters used for aiming error calculations.
      -- OPTIONAL, Type: double, Default: 1.0
      aimingErrorCalcDistanceToTarget = 50; -- the reference distance in meters for defining the max random spread

      -- Required percentage value (0-1) for aiming error calculations.
      -- OPTIONAL, Type: double, Default: 0.0
      requiredAimingErrorPercent = 0.07;  -- minimum percent of max random spread applied under ideal conditions (0 to 1)

      -- Percentage value (0-1) for aiming improvement when prone.
      -- OPTIONAL, Type: double, Default: 1.0
      proneAimingImprovementPercent = 0.2;  -- reduction in random spread while prone, as percent of max spread (0 to 1)

      -- Percentage value (0-1) for general aiming improvement.
      -- OPTIONAL, Type: double, Default: 1.0
      aimingImprovementPercent = 0.2;  -- reuction in random spread while aiming-down-sights, as percentage of max (0 to 1)

      -- Whether to display the compass in the interface.
      -- OPTIONAL, Type: bool, Default: false
      showCompass = true;

      -- Units for compass display ("Degrees" or "NATO-MIL").
      -- OPTIONAL, Type: string, Default: "Degrees"
      -- compassUnits = "NATO-MIL"; -- "Degrees" or "NATO-MIL", defaults to "Degrees"

      -- Input sensitivity multiplier when using binoculars.
      -- OPTIONAL, Type: double, Default: 0.5
      binocularsInputMultiplier = 0.1; -- scales the pitch and yaw input values while using binoculars
   };
  
   -- Menu logic component for the human role
   ["humanMenuLogic"] = {
      componentType = "DtHumanMenuLogic";
      priority = 5;
      --temp values for tasking

      -- Default name for entities to be assigned tasks.
      -- OPTIONAL, Type: string, Default: "shooter"
      taskeeName="shooter";

      -- Default name for entities to be used as shooting targets.
      -- OPTIONAL, Type: string, Default: "target"
      shooteeName="target";

      -- Default name for waypoints in movement tasks.
      -- OPTIONAL, Type: string, Default: "Waypoint 1"
      waypointName="Waypoint 1";
   };

   ["friendlyUnitsLogic"] = {
      componentType = "DtFriendlyUnitsLogic"; 
      priority = 5;
   };
   
   -- Forward declaration of the observer class
   ["humanWeaponPoseLogic"] = {
      componentType = "DtHumanWeaponPoseLogic";
      priority = 5;
   };

   -- Forward declaration of the entity resolver class
   ["humanObserverUpdater"] = { 

      -- Sensor mode name for night vision goggles.
      -- OPTIONAL, Type: string, Default: "NVG"
      -- nightVisionSensorMode = "NVG";

      -- Channel name for scope rendering.
      -- OPTIONAL, Type: string, Default: "RemoteView"
      -- scopeChannelName = "RemoteView";

      -- Pitch angle for scope view when walking.
      -- OPTIONAL, Type: double, Default: -45.0
      -- scopeWalkingPitch = -45.0;
      componentType = "DtHumanObserverUpdater";
      priority = 10;
      
      pitchRate=45; --degrees/second
      maxPitch = 60.0;

      -- Distance threshold for cutting to first-person view.
      -- OPTIONAL, Type: double, Default: 0.4
      cutToFirstPersonDist = 0.6;

      -- Speed of observer position changes when posture changes.
      -- OPTIONAL, Type: double, Default: 2.0
      postureChangeSpeed = 4.0;

      -- Speed of observer movements in third-person aiming mode.
      -- OPTIONAL, Type: double, Default: 9.0
      thirdPersonAimSpeed = 9.0;

      -- Speed of transitions when changing attachment modes.
      -- OPTIONAL, Type: double, Default: 8.0
      attachModeTransitionSpeed = 7.0;

      -- Base position for first person model and camera;
      -- defined as an offset from the entity published position (usually at the characters feet)
      spineEndPosition = {
         x = -0.015372580848634;
         y = -0.055450439453125;
         z = 1.6102104187012;
      };
      cameraPosition = {
         x = -0.180443215678;
         y = -0.117053822199;
         z = 1.883802107666;
      };

      -- Posture/mode offsets (from entity origin) that override cameraPosition;
      -- spineEndPosition is moved to maintain relative offset from base cameraPosition

      -- Table of observer offset configurations for different postures and view modes.
      -- OPTIONAL, Type: table, Default: {}
      offsets={
         firstPersonStanding={
            pos = {
               x = 0.03;
               y = 0.08;
               z = 1.6;
         };
         };
         thirdPersonStanding={
            pos = {
               x = 0.26;
               y = -2.248;
               z = 1.79;
         };
         };
         weaponStanding={
            pos = {
               x = 0.40;
               y = -0.820;
               z = 1.64;
         };
         };
         carryStanding={
            pos = {
               x = 0.26;
               y = -3.248;
               z = 1.79;
         };
         };

         firstPersonCrouching={
            pos = {
               x = 0.02;
               y = 0.21;
               z = 1.35;
         };
         };
         thirdPersonCrouching={
            pos = {
               x = 0.24;
               y = -1.75;
               z = 1.549;
         };
         };
         weaponCrouching={
            pos = {
               x = 0.40;
               y = -0.8200;
               z = 1.644;
         };
         };
         carryCrouching={
            pos = {
               x = 0.24;
               y = -2.75;
               z = 1.549;
         };
         };

         firstPersonKneeling={
            pos = {
               x = 0.02;
               y = 0.21;
               z = 1.35;
         };
         };
         thirdPersonKneeling={
            pos = {
               x = 0.24;
               y = -1.75;
               z = 1.549;
         };
         };
         weaponKneeling={
            pos = {
               x = 0.40;
               y = -0.8200;
               z = 1.644;
         };
         };
         carryKneeling={
            pos = {
               x = 0.24;
               y = -2.75;
               z = 1.549;
         };
         };

         firstPersonProne={
            pos = {
               x = 0.081;
               y = 0.50;
               z = 0.36;
         };
         };
         thirdPersonProne={
            pos = {
               x = 0.199;
               y = -1.50;
               z = 0.678;
         };
         };
         weaponProne={
            pos = {
               x = 0.351;
               y = -0.436;
               z = 0.464;
         };
         };
         carryProne={
            pos = {
               x = 0.259;
               y = -3.25;
               z = 1.79;
         };
         };

         firstPersonSitting={
            pos = {
               x = 0.013;
               y = 0.0;
               z = 1.28;
         };
         };
         thirdPersonSitting={
            pos = {
               x = 0.010;
               y = -1.450;
               z = 1.5490;
         };
         };
         weaponSitting={
            pos = {
               x = 0.400;
               y = -0.820;
               z = 1.644;
         };
         };
         carrySitting={
            pos = {
               x = 0.010;
               y = -1.450;
               z = 1.5490;
         };
         };

         firstPersonJumping={
            pos = {
               x = 0.027;
               y = 0.080;
               z = 1.61;
         };
         };
         thirdPersonJumping={
            pos = {
               x = 0.259;
               y = -4.248;
               z = 1.797;
         };
         };
         weaponJumping={
            pos = {
               x = 0.400;
               y = -0.82;
               z = 1.64;
         };
         };
         carryJumping={
            pos = {
               x = 0.259;
               y = -5.248;
               z = 1.797;
         };
         };

         firstPersonParachuting={
            pos = {
               x = 0.027;
               y = 0.080;
               z = 1.61;
         };
         };
         thirdPersonParachuting={
            pos = {
               x = 0.259;
               y = -20.24;
               z = 5.79;
         };
         };
         weaponParachuting={
            pos = {
               x = 0.400;
               y = -0.82;
               z = 1.645;
         };
         };
         carryParachuting={
            pos = {
               x = 0.259;
               y = -20.24;
               z = 5.79;
         };
      };
      };
      
      -- Name of the sensor mode to use when night vision is enabled.  Allowable values 
      -- for this variable are the key strings defined in the vreSensorModeMapper table, in playerStation.lua.
      nightVisionObserverMode = "NVG";
   };

   -- Forward declaration of the first-person updater agent class
   ["humanRenderUpdater"] = { 
      componentType = "DtHumanRenderUpdater";
      priority = 10;

      -- Duration of hit effects in seconds.
      -- OPTIONAL, Type: double, Default: 4.0
      hitEffectDuration = 3.0;   -- in seconds - the duration of each hit effect

      -- Minimum health value for health effect visibility to prevent screen from going completely opaque.
      -- OPTIONAL, Type: double, Default: 0.15
      healthEffectHealthMinimum = 0.1;  -- a percentage - When 10% of total health is remaining, stop decreasing the transparency because the scene will become not visible otherwise.

      -- Maximum health value for health effect visibility.
      -- OPTIONAL, Type: double, Default: 0.85
      healthEffectHealthMaximum = 0.9;  -- a percentage - When 90% of total health is remainiung, the health effect will be used.  It is a high value because this controls the alpha of the effect.  So,
                                        -- it takes 30% before the effect really starts to be visible.

      -- Maximum opacity value for health effects overlay.
      -- OPTIONAL, Type: double, Default: 0.4
      healthEffectOpacityMaximum = 0.4; -- a percentage - the maximum opacity that the health effect will have.

      -- Whether to hide the first-person player model when aiming. Not applicable in VR mode.
      -- OPTIONAL, Type: bool, Default: false
      hideFirstPersonModelWhileAiming = false;
                                        
      -- Mapping between weapons and model definitions for first-person view.
      -- REQUIRED, Type: DtInitTable
      firstPersonModelMap={
         -- Table key is the name of a first-person Model Definition
         ["Lifeformfirst_person_M4"] = {
            -- guns defines a list of DIGuy hand items for which this first-person model will be used
            guns = {
               "m240";
               "M4A1Carbine";
               "m249";
               "m4_m203";
               "m16_m203";
               "m60";
               "m16";
               "sar_21";
               "XM107SniperRifle";
               "iwi_x95";
               "rifle_k2";
            };
            -- appearances is an optional list of DIGuy appearances for which this first-person model is valid
            appearances={};

            -- pivotJoint is the name of the link in the DIGuy skeleton of the first-person model
            -- that will be used as the base/reference point, as well as the link used to change
            -- the pitch of the model as the player's aim elevation changes.
            pivotJoint = "spine_end";
            -- pitchAxis is the axis of the pivotJoin link that should be used to control the model's pitch
            pitchAxis = "x";

            -- First person model (not camera) offsets
            -- These offsets are used to position the first-person model relative to the point defined by
            -- humanObserverUpdater.spineEndPosition, depending on the chosen player entity posture or 
            -- aiming state. The aiming offset overrides the posture offset when in aim-down-sights mode.
            standingOffset = {
               x = 0;
               y = 0;
               z = 0;
            };
            crouchedOffset = {
               x = 0;
               y = 0;
               z = 0.01;
            };
            proneOffset = {
               x = 0;
               y = 0;
               z = 0.03;
            };
            aimingOffset = {
               x = -0.001;
               y = 0;
               z = 0.0051;
               psi = 0.05;
               theta = 0.0;
               phi = 0.003;
            }; -- allows fine adjustment to line up the iron sights with the scope

            standingOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.2;
            };
            crouchedOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.18;
            };
            proneOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.08;
            };
            aimingOffset_withVr = {
               x = 0.031;
               y = 0;
               z = 0.005;
            }; -- right eye dominant
            --aimingOffset_withVr   = { x=-0.032, y=0, z= 0 }; --left eye dominant
         };

         ["Lifeformfirst_person_ef88"] = {
            guns={ "EF88AusteyrAssaultRifle"};
            appearances={};

            pivotJoint = "spine_end";
            pitchAxis = "x";

            -- First person model (not camera) offsets (from spineEndPosition)
            standingOffset = {
               x = 0;
               y = 0;
               z = 0;
            };
            crouchedOffset = {
               x = 0;
               y = 0;
               z = 0.01;
            };
            proneOffset = {
               x = 0;
               y = 0;
               z = 0.03;
            };
            aimingOffset = {
               x = 0;
               y = 0;
               z = 0.0035;
            }; -- we need to move the butt of the gun (the view) down to line up the iron sights with the scope

            standingOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.2;
            };
            crouchedOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.18;
            };
            proneOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.08;
            };
            aimingOffset_withVr = {
               x = 0.031;
               y = 0;
               z = 0.005;
            }; -- right eye dominant
            --aimingOffset_withVr   = { x=-0.032, y=0, z= 0 }; --left eye dominant
         };

         ["Lifeformfirst_person_ak47"] = {
            guns={ "ak47" };
            appearances= {};

            pivotJoint = "spine_end";
            pitchAxis = "x";

            -- First person model (not camera) offsets (from spineEndPosition)
            standingOffset = {
               x = 0;
               y = 0;
               z = 0;
            };
            crouchedOffset = {
               x = 0;
               y = 0;
               z = 0.01;
            };
            proneOffset = {
               x = 0;
               y = 0;
               z = 0.03;
            };
            aimingOffset = {
               x = 0;
               y = 0;
               z = 0;
            };

            standingOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.2;
            };
            crouchedOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.18;
            };
            proneOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.08;
            };
            aimingOffset_withVr = {
               x = 0.032;
               y = 0;
               z = 0;
            }; -- right eye dominant
            --aimingOffset_withVr   = { x=-0.032, y=0, z= 0 }; --left eye dominant
         };

         ["Lifeformfirst_person_m9"] = {
            guns={ "m9"; "pistol_k5"; };
            appearances={};

            pivotJoint = "spine_end";
            pitchAxis = "x";

            -- First person model (not camera) offsets (from spineEndPosition)
            standingOffset = {
               x = 0;
               y = 0;
               z = -0.04;
            };
            crouchedOffset = {
               x = 0;
               y = 0;
               z = -0.03;
            };
            proneOffset = {
               x = 0;
               y = 0;
               z = 0.0;
            };
            aimingOffset = {
               x = -0.001;
               y = 0;
               z = 0;
            };

            standingOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.2;
            };
            crouchedOffset_withVr = {
               x = 0;
               y = 0;
               z = -0.18;
            };
            proneOffset_withVr = {
               x = 0;
               y = -0.06;
               z = -0.08;
            };
            aimingOffset_withVr = {
               x = 0.032;
               y = 0;
               z = -0.005;
            }; -- right eye dominant
            --aimingOffset_withVr   = { x=-0.032, y=0, z= 0 }; --left eye dominant
         };

         ["NO_MODEL"] = {
            guns = {"no_weapon"; "grenade"};
            appearances = {};
         };
      };
   };

   -- Component for managing human-specific audio effects
   ["humanAudioUpdater"] = {
      componentType = "DtHumanAudioUpdater";
      priority = 10;
      mm556="m16-single-shot-fixed.wav";
      mm556Type="2:8:225:2:1:1:0";

      -- Name of the soundscape configuration file for ambient audio.
      -- OPTIONAL, Type: string, Default: ""
      soundscape="";

      -- Volume level for the ambient soundscape audio.
      -- OPTIONAL, Type: double, Default: 0.7
      soundscapeVolume = 0.8;
      
      -- Configuration table containing audio settings for parachute and weapon sounds.
      -- OPTIONAL, Type: table, Default: {}
      soundConfig = {
         freeFall = { 
            filename = "parachuteAmbient.wav";
            volume = 1.0;
            pitch = 1.0;
            normalSpeed = 100.0;
            loop = true;
            is3d = false;
         };
         open = { 
            filename = "parachuteOpen.wav";
            volume = 1.0;
            pitch = 1.0;
            loop = false;
            is3d = false;
         };
         embarked = {
            filename = "parachuteInterior.wav";
            pitch = 1.0;
            loop = true;
            is3d = true;
            decibels = 85.0;
         };
         reload = {
            filename = "change_weapon_1.wav";
            pitch = 1.0;
            loop = false;
            is3d = false;
            decibels = 85.0;
         };
      };
   };
};

connectors = {"DtFriendlyUnitsConnector"};

includes ={
   "@(vre-roles-dir)/devices/radio.human.lua";
   "@(vre-roles-dir)/devices/minimap.lua";
   "@(vre-roles-dir)/devices/binoculars.lua";
   "@(vre-roles-dir)/devices/scope.lua";
   "@(vre-roles-dir)/devices/deployableObjects.lua";
   "@(vre-roles-dir)/menus/human.lua";
   "@(vre-roles-dir)/menus/quickMenus.lua";
   "@(vre-roles-dir)/menus/quickmenus.human.lua";
};
