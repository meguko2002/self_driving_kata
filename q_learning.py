# objective is to get the cart to the flag.
# for now, let's just move randomly:
import numpy as np

LEARNING_RATE = 0.1

DISCOUNT = 0.95
EPISODES = 25000
SHOW_EVERY = 3000

DISCRETE_OS_SIZE = [3, 3, 3, 3]

# Exploration settings
epsilon = 1  # not a constant, qoing to be decayed
START_EPSILON_DECAYING = 1
END_EPSILON_DECAYING = EPISODES//2
epsilon_decay_value = epsilon/(END_EPSILON_DECAYING - START_EPSILON_DECAYING)
action_space_n = 8
X_LIM = 800
Y_LIM = 600
XY_RATO = 0.25
V_LIM = 1000   # 仮値
q_table = np.random.uniform(low=-2, high=0, size=(DISCRETE_OS_SIZE + [action_space_n]))


def get_discrete_state(state):
    if state[0] < X_LIM * XY_RATO:
        discrete_state[0] = 0
    elif state[0] < X_LIM * (1 -XY_RATO):
        discrete_state[0] = 1
    else:
        discrete_state[0] = 2

    if state[1] < Y_LIM * XY_RATO:
        discrete_state[1] = 0
    elif state[1] < Y_LIM * (1 - XY_RATO):
        discrete_state[1] = 1
    else:
        discrete_state[1] = 2

    for i in (2, 3):
        if state[i] < - V_LIM:
            discrete_state[i] = 0
        elif state[i] < V_LIM:
            discrete_state[i] = 1
        else:
            discrete_state[i] = 2
    return tuple(discrete_state)


for episode in range(EPISODES):
    discrete_state = get_discrete_state(env.reset())
    done = False

    if episode % SHOW_EVERY == 0:
        render = True
        print(episode)
    else:
        render = False

    while not done:

        if np.random.random() > epsilon:
            # Get action from Q table
            action = np.argmax(q_table[discrete_state])
        else:
            # Get random action
            action = np.random.randint(0, env.action_space.n)


        new_state, reward, done, _ = env.step(action)

        new_discrete_state = get_discrete_state(new_state)

        if episode % SHOW_EVERY == 0:
            env.render()
        #new_q = (1 - LEARNING_RATE) * current_q + LEARNING_RATE * (reward + DISCOUNT * max_future_q)

        # If simulation did not end yet after last step - update Q table
        if not done:

            # Maximum possible Q value in next step (for new state)
            max_future_q = np.max(q_table[new_discrete_state])

            # Current Q value (for current state and performed action)
            current_q = q_table[discrete_state + (action,)]

            # And here's our equation for a new Q value for current state and action
            new_q = (1 - LEARNING_RATE) * current_q + LEARNING_RATE * (reward + DISCOUNT * max_future_q)

            # Update Q table with new Q value
            q_table[discrete_state + (action,)] = new_q


        # Simulation ended (for any reson) - if goal position is achived - update Q value with reward directly
        elif new_state[0] >= env.goal_position:
            #q_table[discrete_state + (action,)] = reward
            q_table[discrete_state + (action,)] = 0

        discrete_state = new_discrete_state

    # Decaying is being done every episode if episode number is within decaying range
    if END_EPSILON_DECAYING >= episode >= START_EPSILON_DECAYING:
        epsilon -= epsilon_decay_value


env.close()
