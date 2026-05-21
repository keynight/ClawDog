# ADR-003: Use OpenClaw Instead of Custom Gait Generation

## Status
Accepted

## Date
2025-01-20

## Context
The ClawDog robot is a quadruped with 4 legs (1 DOF per leg). It requires:
- Multiple gait patterns (stand, walk, trot)
- Stable locomotion on flat and uneven surfaces
- Integration with ROS2 navigation (Nav2)
- Potential for AI-driven high-level commands

We must decide whether to implement custom gait generation or use an existing framework.

## Decision
Use OpenClaw for quadruped gait generation and locomotion control.

OpenClaw provides:
- Pre-implemented gait generators (stand, walk, trot, bound)
- ROS2 integration packages
- Stable, community-tested algorithms
- Parameter tuning for different robot morphologies

## Alternatives Considered

### Custom Gait Implementation
- **Pros**: Full control over algorithms, optimized for our specific hardware
- **Cons**: 
  - Requires significant robotics expertise and development time
  - Must handle stability, terrain adaptation, and recovery behaviors
  - No community support or prior validation
  - Risk of poor locomotion performance
- **Rejected**: Time and expertise requirements exceed project scope

### Other Quadruped Frameworks
- **Champ**: ROS2 quadruped framework, but less focused on AI integration
- **SpotMicro**: Specific to SpotMicro robot design, not easily adaptable
- **Pupper**: Educational framework, limited gait variety
- **Rejected**: OpenClaw is the most mature option with explicit AI agent integration (RosClaw)

### Direct Motor Control (No Gait Framework)
- **Pros**: Simplest implementation
- **Cons**: Robot would only be capable of pre-programmed motions
- **Rejected**: Cannot achieve autonomous locomotion or adapt to terrain

## Consequences

**Positive:**
- Rapid development - gait generation is ready to use
- Community-tested stability algorithms
- ROS2 integration available out-of-the-box
- Clear path to AI integration via RosClaw
- Can focus on hardware integration rather than locomotion theory

**Negative:**
- Dependency on external project (maintenance risk)
- May need to adapt OpenClaw's URDF/model to our specific robot dimensions
- Less flexibility to implement novel gait patterns
- Must learn OpenClaw's configuration and API

**Mitigations:**
- Fork OpenClaw repository for stability
- Document any modifications needed for ClawDog hardware
- Implement fallback static poses in ESP32 firmware (sit, stand) independent of OpenClaw
- Monitor OpenClaw project health and have migration plan if needed

## References
- OpenClaw repository: https://github.com/openclaw
- DESC.md section 3.2 and 3.3 for OpenClaw and RosClaw integration
- README.md Phase 3 and 4 for architecture overview
