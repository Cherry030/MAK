# SharedData 구성

##    DI-Guy 캐릭터 리소스
-----
### 기본 Human Entity 리소스 경로   
###  → (SharedData)/19/latest/ModelData/Lifeforms/DIGuy  

#### Folder  
#### 1. config   
>  - diguy/char_soldier_17_action_table.cfg  
>    : Custom 애니메이션, 제스처 파일(bdm)추가   
>      char_soldier_17 타입의 객체들이 사용할 동작이 정리된 config    
>         * ROK 수신호, 애니메이션 추가

 2. custom  
>  - config  
>    :  Custom 리소스(캐릭터, 무기, 도구 등) Config  
>  - custom  
>    : Custom 리소스(캐릭터, 무기, 도구 등) Config  
>  - motion  
>    : Custom 리소스(캐릭터, 무기, 도구 등) Config  

 3. geometry  
>  - optimized_geometry_cache  
>    : DI-Guy에서 사용한 캐릭터가 사용하는 리소스의 압축 버전  
>      DI-Guy Character Appearance에서 사용하는 dae파일(3D 모델)의 캐시 파일  
>      해당 파일이 없을 시, DI-Guy Character Viewer에서 오류 발생  
>        * male_w1_head_dae.bdg     : 머리 리소스  
>        * multicam_1_dae.bdg       : 몸통 리소스  
>        * sk_felin_helmet_dae.bdg  : 헬멧 리소스  

#### 업로드 시  
#### 1. config 업로드  
>        <char_soldier_17_action_table.cfg>  
>        1) 추가하자고자하는 새 action name을 리스트 아래 추가
>         ```c
>           action_table soldier_17  
>           action_name = stand_ready  
>           ...
>         ```
>          	**추가**  
>         2) 각 action table마다 동일한 entry 추가 ${\textsf{\color{red}(필수!)}}$   
>             *** 각 Table마다 갯수가 동일하지 않으면 오류 발생**
>         ```c
>         	#
>          #  greg_mortar_kneel_ready
>         	#
>          action_table_entry = null
>            ...
>         ```
>         3) 실제 bdm파일과 애니메이션 시작/종료 시점 등 설정  
>         ```c  
>        	motion_arc bill_greg_stand_17        :   원하는 애니메이션 이름  
>       		datafile = bill_greg_stand_17.bdm    :   실제 bdm 파일 명  
>       		actor = bill  
>       		tin = 0.99  
>       		tout = 2.01  
>       		travel_direction_overall = none  
>       		facing_direction_tin = inherit  
>       		facing_direction_tout = inherit  
>       		link_target = None  
>       		auto_calc = false  
>            ...  
>         ```  
>>            3-1) 한 애니메이션 종류에 들고있는 HandItem 타입 별 bdm 설정 자동 변환  
>>            ```c  
>>             selector_data  
>>                 selector  
>>                 arc_name = greg_suspect_09_walk_head_low
>>                 metadata = no_weapon          :   원하는 HandItem 타입명
>>                 metadata_val_0 = 0
>>                 metadata_val_2 = 1
>>            ```       
#### 2. custom 업로드
