# Anim Blueprint Setup Guide for ABP_Rabbit

## Correct Workflow

### Step 1: Create the Anim Blueprint
1. Right-click in Content Browser → **Animation → Anim Blueprint**
2. Select **Anim Instance** as parent class
3. Select **SK_Rabbit** skeleton
4. Name it: **ABP_Rabbit**
5. Save at: `/Game/EndlessRunner/Characters/`

### Step 2: Set Up State Machine (Anim Graph)

1. **Open Anim Graph** (not Event Graph)
2. **Add State Machine node**:
   - Drag from empty space → search "State Machine"
   - Name it "MovementStateMachine"
   - Connect its output to **Output Pose**

3. **Double-click State Machine** to open it

4. **Create 3 States**:
   - Right-click in state machine → **Add State**
   - Name them:
     - `Running` (set as Entry/Default)
     - `Jumping`
     - `Ducking`

5. **In Each State, Add Animation Sequence**:
   
   **Running State:**
   - Double-click `Running` state
   - Drag from empty space → search "Play Animation Sequence"
   - Select `ANIM_Rabbit_Walk`
   - Connect to **Output Pose**
   
   **Jumping State:**
   - Double-click `Jumping` state
   - Drag from empty space → search "Play Animation Sequence"
   - Select `ANIM_Rabbit_1Hop`
   - Connect to **Output Pose**
   
   **Ducking State:**
   - Double-click `Ducking` state
   - Drag from empty space → search "Play Animation Sequence"
   - Select `ANIM_Rabbit_IdleSatBreathe`
   - Connect to **Output Pose**

### Step 3: Set Up Transitions (In State Machine)

1. **Create Transitions**:
   - Click and drag from `Running` state → `Jumping` state (creates transition)
   - Click and drag from `Jumping` state → `Running` state
   - Click and drag from `Running` state → `Ducking` state
   - Click and drag from `Ducking` state → `Running` state
   - Click and drag from `Jumping` state → `Running` state (when landing)

2. **Set Up Transition Rules** (using AnimationState):

   **For Running → Jumping transition:**
   - Click on the transition arrow
   - In Details panel, find **Transition Rules**
   - Click **+ Add Rule**
   - In the graph that opens:
     - Add **Get AnimationState** node (from RabbitCharacter)
     - Add **Equal (Enum)** node
     - Set enum value to **Jumping**
     - Connect: Get AnimationState → Equal → Transition Rule Result

   **For Jumping → Running transition:**
   - Click on the transition arrow
   - Add rule: AnimationState == Running

   **For Running → Ducking transition:**
   - Click on the transition arrow
   - Add rule: AnimationState == Ducking

   **For Ducking → Running transition:**
   - Click on the transition arrow
   - Add rule: AnimationState == Running

### Step 4: Get AnimationState in State Machine

The State Machine needs access to the character's AnimationState. You have two options:

**Option A: Use Event Graph (Recommended)**
1. Go to **Event Graph** tab
2. Add **Event Blueprint Update Animation** node
3. Add **Try Get Pawn Owner** node
4. Cast to **RabbitCharacter**
5. Get **AnimationState** property
6. Store in a variable (e.g., `CurrentAnimationState`)

**Option B: Direct Access in State Machine**
1. In State Machine graph, add **Try Get Pawn Owner**
2. Cast to **RabbitCharacter**
3. Get **AnimationState**
4. Use in transition rules

### Step 5: Assign to Character

1. Open **BP_RabbitCharacter**
2. In Details panel, find **Animation** section
3. Set **Anim Class** to `ABP_Rabbit`

## Summary

- ✅ **Animations go IN the states** (not in Event Graph)
- ✅ **State Machine handles the switching** (not Switch on Enum)
- ✅ **Transition rules check AnimationState** to determine when to switch
- ✅ **Each state has its own Animation Sequence node**

The State Machine is the right place for animations - it's designed exactly for this!

