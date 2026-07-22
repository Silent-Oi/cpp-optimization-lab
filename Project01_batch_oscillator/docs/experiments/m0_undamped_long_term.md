# 谐振子时间步积分长期行为记录

## 1. 20260722

omega = 2*pi
T = 1
initial_state = {1, 0}
dt = T/1000
总时间 = 123.456T
总步数 = 123456

### 精确解

position: -0.962028
velocity: -1.71501
energy: 19.7392

### explicit_euler

explicit_euler_position: -10.9705
explicit_euler_velocity: -20.3196
explicit_euler_energy: 2582.1
error_explicit_euler_position: 10.0085
error_explicit_euler_velocity: 18.6046
error_explicit_euler_energy: -2562.36

### symplectic_euler

symplectic_euler_position: -0.963229
symplectic_euler_velocity: -1.7073
symplectic_euler_energy: 19.7717
error_symplectic_euler_position: 0.00120115
error_symplectic_euler_velocity: -0.00770573
error_symplectic_euler_energy: -0.0324616

### velocity_verlet

velocity_verlet_position: -0.962375
velocity_verlet_velocity: -1.70729
velocity_verlet_energy: 19.7392
error_velocity_verlet_position: 0.000347497
error_velocity_verlet_velocity: -0.00772258
error_velocity_verlet_energy: 1.43842e-05

### position_verlet

position_verlet_position: -0.962375
position_verlet_velocity: -1.70729
position_verlet_energy: 19.7392
error_position_verlet_position: 0.000347497
error_position_verlet_velocity: -0.00772258
error_position_verlet_energy: 1.43841e-05
