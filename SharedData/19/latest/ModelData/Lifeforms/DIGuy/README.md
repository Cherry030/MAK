# SharedData 구성

##    DI-Guy 캐릭터 리소스
-----
### 기본 Human Entity 리소스 경로 
###  : (SharedData)/19/latest/ModelData/Lifeforms/DIGuy

#### Folder
#### 1. config 
####>>- diguy/char_soldier_17_action_table.cfg
####>>>>: Custom 애니메이션, 제스처 파일(bdm)추가 
####>>>>  char_soldier_17 타입의 객체들이 사용할 동작이 정리된 config
####>>>>>>  * ROK 수신호, 애니메이션 추가

#### 2. custom
####>>- config
####>>>>:  Custom 리소스(캐릭터, 무기, 도구 등) Config
####>>- config
####>>>>: Custom 리소스(캐릭터, 무기, 도구 등) Config
####>>- motion
####>>>>: Custom 리소스(캐릭터, 무기, 도구 등) Config

#### 3. geometry
####>>- optimized_geometry_cache
####>>>>: DI-Guy에서 사용한 캐릭터가 사용하는 리소스의 압축 버전
####>>>>  DI-Guy Character Appearance에서 사용하는 dae파일(3D 모델)의 캐시 파일
####>>>>  해당 파일이 없을 시, DI-Guy Character Viewer에서 오류 발생
####>>>>>>  * male_w1_head_dae.bdg     : 머리 리소스
####>>>>>>  * multicam_1_dae.bdg       : 몸통 리소스
####>>>>>>  * sk_felin_helmet_dae.bdg  : 헬멧 리소스
