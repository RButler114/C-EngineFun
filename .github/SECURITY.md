# Security Policy

## Supported Versions

We currently support the following versions with security updates:

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |
| < 1.0   | :x:                |

## Reporting a Vulnerability

We take security seriously. If you discover a security vulnerability in the C++ Game Engine, please report it responsibly.

### How to Report

1. **Do NOT create a public GitHub issue** for security vulnerabilities
2. **Email us directly** at: [your-email@example.com] (replace with your actual email)
3. **Include the following information**:
   - Description of the vulnerability
   - Steps to reproduce the issue
   - Potential impact
   - Suggested fix (if you have one)

### What to Expect

- **Acknowledgment**: We'll acknowledge receipt within 48 hours
- **Assessment**: We'll assess the vulnerability within 5 business days
- **Updates**: We'll provide regular updates on our progress
- **Resolution**: We'll work to resolve critical issues within 30 days

### Responsible Disclosure

- Please give us reasonable time to fix the issue before public disclosure
- We'll credit you in our security advisories (unless you prefer to remain anonymous)
- We may offer recognition for significant security contributions

## Security Best Practices

When using this game engine:

1. **Keep dependencies updated**: Regularly update SDL2 and other dependencies
2. **Validate input**: Always validate user input and file formats
3. **Use latest version**: Use the most recent stable version of the engine
4. **Review code**: Review any custom game code for security issues

## Security Features

The engine includes these security considerations:

- **Memory safety**: Uses RAII and smart pointers to prevent memory leaks
- **Input validation**: Basic input sanitization in the input manager
- **Resource management**: Proper cleanup of SDL2 resources

## Contact

For security-related questions or concerns:
- Email: [your-email@example.com]
- GitHub: [@RButler114](https://github.com/RButler114)

Thank you for helping keep the C++ Game Engine secure!
