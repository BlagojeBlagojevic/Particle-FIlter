# Particle Filter


## 1. Introduction

This paper describes in detail the theoretical foundations and practical implementation of the particle filter (Particle Filter), also known as Monte Carlo Localization (MCL), for solving 3D positioning problems.

The problem is reduced to estimating the position (x,y,z) of an object in space based on noisy distance measurements from three fixed transmitters (beacons) with known locations.

The attached C code, which implements the complete SIR (Sequential Importance Resampling) algorithm, including prediction, weight update, and resampling steps, is analyzed.

The paper explains how the probabilistic representation of the system's state (position) is approximated using a set of weighted particles and how this set is updated over time to track the actual position of the object.

---

## 2. Monte Carlo Methods and Markov Chains

### 2.1 Introduction to Monte Carlo Methods

Monte Carlo (MC) methods represent a broad class of computational algorithms that rely on repeated random sampling to obtain numerical results.

Their fundamental idea is that a problem, which is often deterministic or too complex for analytical solving, is modeled or simulated using stochastic (random) processes.

One of the most common applications of MC methods is the estimation (approximation) of definite integrals, especially in high-dimensional spaces where traditional numerical methods (like the trapezoidal rule) are inefficient.

Suppose we want to calculate the integral of a function $g(x)$ over the domain D:

$$I=\int_{D}g(x)dx$$

If we rewrite the integral as the expected value of a function $f(x)$ with respect to some probability distribution $p(x)$

$$I=\int_{D}f(x)p(x)dx=E_{p}[f(x)]$$

where $f(x)=g(x)/p(x)$ (if $p(x)>0$ for all $x\in D$), then the integral can be approximated by the average of the function $f(x)$ values sampled from the distribution $p(x)$:

$$I_{N}=\frac{1}{N}\sum_{i=1}^{N}f(X_i)$$

where $X_i$ are independent samples from $p(x).$ By the Law of Large Numbers, as $N\rightarrow\infty$, the approximation $I_{N}$ converges to the true value I.

### 2.2 Markov Chains

A Markov chain is a stochastic process that models a sequence of possible events in which the probability of transitioning from one state to another depends only on the current state, and not on the sequence of events that preceded it.

This is called the Markov property.

Mathematically, for a sequence of states $X_1, X_2, X_3, ...$

$$P[X_{n+1}=x | X_1, X_2, ..., X_n]=P[X_{n+1}=x | X_n]$$

The key property of Markov chains in the context of sampling is their ability, under certain conditions (irreducibility and aperiodicity), to converge to a stationary distribution ($\pi$).

If the chain has a stationary distribution $\pi$, it means that if the state distribution at some point is $\pi$, it will also be $\pi$ at the next point: $\pi P=\pi$, where P is the transition matrix.

If the chain "runs" long enough (burn-in period), the samples generated after this period can be considered samples coming from $\pi$.

### 2.3 Markov Chain Monte Carlo (MCMC)

MCMC methods combine the ideas of Monte Carlo simulation and Markov chains.

They are primarily used when we want to sample from a complex probability distribution $P(\theta)$ that cannot be easily sampled directly.

This is typical for Bayesian inference, where $P(\theta)$ is the posterior distribution.

The MCMC algorithm constructs a Markov chain whose stationary distribution is exactly the target distribution $P(\theta)$.

After the chain is run and reaches convergence (after the burn-in period), the generated sequence of states $\theta^{(1)}, \theta^{(2)}, ..., \theta^{(N)}$ represents samples from the desired distribution $P(\theta)$.

We then use these samples to estimate expected values, quantiles, or other characteristics of the $P(\theta)$ distribution, using the classic MC approach.

### 2.4 Key MCMC Algorithms

**a) Metropolis-Hastings Algorithm**

This is the most basic and most important MCMC algorithm. It works by proposing a new state $$\theta^{\*}$$ from a proposal distribution $Q(\theta^{*}|\theta^{(t)})$ and accepts that state with an acceptance probability A:

$$A=min\left(1, \frac{P(\theta^{\*})Q(\theta^{(t)}|\theta^{\*})}{P(\theta^{(t)})Q(\theta^{\*}|\theta^{(t)})}\right)$$

If $\theta^{*}$ is accepted, $\theta^{(t+1)}=\theta^{\*}$; otherwise, $\theta^{(t+1)}=\theta^{(t)}$. The algorithm guarantees that the stationary distribution of the chain will be the target distribution $P(\theta)$.

**b) Gibbs Sampler**

A special case of the Metropolis-Hastings algorithm, the Gibbs sampler, is used when the target distribution is high-dimensional, but the conditional distributions for each dimension are known and easy to sample from.

The Gibbs sampler works by iteratively sampling each component $\theta_j$ from its conditional distribution, keeping the other components fixed at their current values:

$$\theta_{j}^{\(t+1)}\sim P(\theta_{\j} | \theta_{1}^{\(t+1)}, ..., \theta_{j-1}^{\(t+1)}, \theta_{j+1}^{\(t)}, ..., \theta_{k}^{\(t)})$$

---

## 3. Theoretical Foundations of Particle Filters

Particle filters, also known as sequential Monte Carlo methods, represent a set of Monte Carlo algorithms used to find approximate solutions to filtering problems for nonlinear state-space systems, such as in signal processing and Bayesian statistical inference.

The filtering problem consists of estimating the internal states in dynamic systems when partial observations are made and when random perturbations are present in the sensors, as well as in the dynamic system.

The goal is to calculate the posterior distributions of the states of a Markov process, given noisy and partial observations.

Particle filtering uses a set of particles (also called samples) to represent the posterior distribution of a stochastic process given noisy and/or partial observations.

The state-space model can be nonlinear, and the initial state and noise distributions can take any required form.

Particle filter techniques provide a well-established methodology for generating samples from the required distribution without needing assumptions about the state-space model or the state distribution.

However, these methods do not perform well when applied to very high-dimensional systems.

Particle filters update their predictions in an approximate (statistical) manner. Samples from the distribution are represented by a set of particles; each particle is assigned a probability weight that represents the probability that this particle will be sampled from the probability density function.

Weight inequality leading to weight collapse is a common problem encountered in these filtering algorithms.

However, this can be mitigated by including a resampling step before the weights become too unequal.

Several adaptive resampling criteria can be used, including the variance of the weights and the relative entropy with respect to the uniform distribution.

In the resampling step, particles with negligible weights are replaced by new particles in the vicinity of particles with higher weights.

The particle filter methodology is used to solve Hidden Markov Model (HMM) and nonlinear filtering problems.

With the notable exception of linear-Gaussian signal observation models (Kalman filter) or broader classes of models (Beneš filter), Mireille Chaleyat-Maurel and Dominique Michel proved in 1984 that the sequence of posterior distributions of the random states of the signal, given the observations (i.e., the optimal filter), does not have a finite recursion.

Various other numerical methods based on fixed-grid approximations, Markov Chain Monte Carlo techniques, conventional linearization, extended Kalman filters, or determining the best linear system (in terms of the expected cost-error ratio) are unable to cope with large-scale systems, unstable processes, or insufficiently smooth nonlinearities.

Particle filters find application in signal and image processing, Bayesian inference, machine learning, risk analysis and rare event sampling, engineering and robotics, artificial intelligence, bioinformatics, phylogenetics, computer science, economics and mathematical finance, molecular chemistry, computational physics, pharmacokinetics, quantitative risk and insurance, and other fields.

---

## 4. Problem Model and Mathematical Formulation

The problem considered in this paper relates to estimating the three-dimensional position of an object based on the distance from three known transmitters (beacons).

Each beacon emits a signal that propagates through space, and the object receives this signal and estimates the distance to the beacon based on signal strength or propagation time.

The device being positioned is at an unknown coordinate $(x,y,z)$, while the beacon positions are known and denoted as $(x_i, y_i, z_i)$, where $i=1, 2, 3$.

The distance measurement $d_i$ can be written as:

$$d_{i}=\sqrt{(x-x_{i})^2+(y-y_{i})^2+(z-z_{i})^2}+e_{i}$$

(1)

where $e_i$ represents measurement noise, usually modeled by a Gaussian distribution with a mean of 0 and variance $\sigma^2$. Such a problem is nonlinear due to the quadratic relationship between coordinates and distance, so classic linear filters (e.g., Kalman filter) cannot be directly applied.

Instead, a Particle Filter is used, which approaches the problem probabilistically, approximating the posterior distribution of the position with a set of discrete particles.

Each particle represents one possible estimate of the object's position.

**Algorithm 1: Particle Filter Algorithm**

* **Data:** $S_{t-1}=\{x_{t-1}^{j},w_{t-1}^{j}\}$
* **Initialization:** $S_{t}=\emptyset$, $\eta=0$;
* **for** $i=1:n$ **do**
    * Sample index $j(i)$ from the discrete distribution given by $w_{t-1}$;
    * Sample $x_{t}^{i}$ from $P(X_{t}|X_{t-1},u_{t})$ using $X_{t-1}^{j}(i)$ and $u_{t}$;
    * $w_{t}^{i}=P(z_{t}|X_{t}^{i})$;
    * $\eta=\eta+w_{t}^{i}$;
    * $S_{t}=S_{t}\cup\{x_{t}^{i},w_{t}^{i}\}$;
* **end**
* **for** $i=1:n$ **do**
    * $w_{t}^{i}=\frac{w_{t}^{i}}{\eta}$;
* **end**

### 4.1 Particle Filter Cycle (SIR Algorithm)

The most common type of particle filter is SIR (Sequential Importance Resampling), which consists of three key steps that are repeated for each new measurement:

1.  **Prediction:** Each particle $x_{k-1}^{i}$ is moved according to the motion model to obtain a new hypothesis $x_{k}^{i}$. Process noise is usually added to this step to simulate uncertainty in movement. $x_{k}^{i}\sim p(x_{k} | x_{k-1}^{i})$

2.  **Update:** For each new hypothesis $x_{k}^{i}$, its weight $w_{k}^{i}$ is calculated. The weight represents how "well" that hypothesis agrees with the actual measurement $z_{k}$. The weight is calculated using the measurement model (likelihood function): $w_{k}^{i}=w_{k-1}^{i}\cdot p(z_{k} | x_{k}^{i})$ After calculation, the weights are normalized so that their sum is 1: $w_{k}^{i}=w_{k}^{i} / \sum_{j} w_{k}^{j}$.

3.  **Resampling:** This step solves the particle degeneracy problem, where over time most particles get negligibly small weights, and only a few particles carry all the probability. In this step, a new set of N particles is created by selecting particles from the current set with a probability proportional to their weights. Particles with high weights are likely to be selected multiple times, while those with low weights will be discarded. After this, all particles in the new set have an equal weight $(1/N)$.

After these three steps, the final state estimation is most often obtained as a weighted average of all particles: $x_{k}=\sum_{i} w_{k}^{i} \cdot x_{k}^{i}$.

---

## 5. Implementation in C Programming Language

The attached C code represents a concrete, low-level implementation of the Sequential Importance Resampling (SIR) algorithm, making the Particle Filter an operational solution for 3D localization.

The structural basis of the implementation rests on defining the system's state: the particle (Particle), which carries 3D coordinates and an associated weight, the reference point (Beacon) with fixed coordinates, and the main container (ParticleFilter3D), which stores the entire state of the filter, including a dynamic array of particles, known beacon positions, key noise parameters ($\sigma_{meas}$, $\sigma_{pro}$), and the state of the random number generator.

The use of optimized functions for Gaussian noise (rand\_GAUS) and Euclidean distance (distance\_3d) emphasizes the focus on performance.

The filter initialization, carried out by the init\_PF3D function, sets the system parameters and then generates N particles, distributing them uniformly throughout the defined 3D workspace.

Each of these initial hypotheses is given an equal weight, $w=1/N$.

The core of the algorithm is implemented in the step\_PF3D function, which iteratively processes incoming measurements.

Each cycle begins with the prediction phase, where each particle is perturbed by adding Gaussian noise, with deviation $\sigma_{pro}$, to its coordinates, simulating the uncontrolled uncertainty of the object's movement, after which the particles are kept within physical boundaries using the CLAMP mechanism.

Prediction is followed by the weight update phase, which is key for Bayesian inference.

The filter calculates the predicted distance of each particle to the beacons and compares it with the actually measured distances.

The total squared error is then used within the Gaussian likelihood function, whose deviation $\sigma_{meas}$ models the measurement noise, to calculate the particle's new ponder (weight).

These weights are then normalized so that their sum equals one.

Finally, the resampling phase solves the particle degeneracy problem. When the effective number of particles drops below a threshold, the Systematic Sampling algorithm is activated.

This multiplies particles with higher probability (higher weight), while those far from the true position are eliminated, thus concentrating computational resources on the most relevant hypotheses, and all new particles receive equal weights.

The final estimation of the object's position is achieved by the estimate\_PF3D function, by calculating the weighted average of all particles, representing the best state estimate.

The main function (main) integrates these steps into a simulation loop, recording the filter's performance and outputting data for subsequent visualization using a Python script.

---

