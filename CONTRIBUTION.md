# Contributing to Inertia Simulation

Welcome to the Inertia Simulation project! To keep our codebase clean and our Git history sane, please follow these rules strictly.

## 🚨 The Golden Rules

1.  **NEVER push directly to the `main` branch.**
    * Always work on a separate branch.
    * Direct commits to `main` will be rejected.
2.  **NEVER use `git push --force`.**
    * This destroys history. If you have a conflict, ask for help.
3.  **Pull before you start.**
    * Always run `git pull origin main` before creating a new branch.

---

## 🛠 Workflow (How to contribute)

1.  **Sync your repo:**
    ```bash
    git checkout main
    git pull origin main
    ```

2.  **Create a new branch:**
    * Use a descriptive name.
    * Format: `type/description`
    * Examples:
        * `feature/g-force-calc`
        * `fix/chair-vibration`
        * `docs/readme-update`
    ```bash
    git checkout -b feature/my-new-feature
    ```

3.  **Code & Commit:**
    * We follow **Conventional Commits**.
    * Format: `type: description`
    * Examples:
        * `feat: add basic physics engine loop`
        * `fix: resolve UART communication timeout`
        * `style: reformat code to Allman style`

4.  **Push & Pull Request:**
    ```bash
    git push origin feature/my-new-feature
    ```
    * Go to GitHub and create a **Pull Request (PR)** to `main`.
    * **Do not merge your own PR.** Wait for a code review.

---

## 🎨 Coding Style (C++)

We use the **Allman Style** for braces. This is non-negotiable.

**✅ DO THIS (Allman):**
```cpp
void CalculateGForce()
{
    if (speed > 100)
    {
        ApplyForce();
    }
    else
    {
        ReduceForce();
    }
}
```
**❌ DO NOT DO THIS (K&R):**

```cpp
void CalculateGForce() {  // NO!
    if (speed > 100) {    // NO!
        ApplyForce();
    } else {              // NO!
        ReduceForce();
    }
}
```

**📁 File Structure**
* Do not commit build files (.exe, .o, .obj, build/).
* Only commit source code (.cpp, .h) and assets.