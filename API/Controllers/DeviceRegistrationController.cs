
using API.Data;
using DomainModels;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using System.Security.Claims;

namespace API.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    [Authorize]
    public class DeviceRegistrationController : ControllerBase
    {
        private readonly AppDBContext _ctx;
        
        public DeviceRegistrationController(AppDBContext ctx) => _ctx = ctx;

        // GET: api/DeviceRegistration
        [HttpGet]
        public async Task<ActionResult<IEnumerable<DeviceRegistration>>> GetUserDevices()
        {
            var userId = GetCurrentUserId();
            if (userId == null) return Unauthorized();

            var devices = await _ctx.DeviceRegistrations
                .Where(d => d.UserId == userId.Value)
                .OrderByDescending(d => d.RegisteredAt)
                .ToListAsync();

            return Ok(devices);
        }

        // POST: api/DeviceRegistration
        [HttpPost]
        public async Task<ActionResult<DeviceRegistration>> RegisterDevice([FromBody] RegisterDeviceRequest request)
        {
            var userId = GetCurrentUserId();
            if (userId == null) return Unauthorized();

            if (string.IsNullOrWhiteSpace(request.DeviceId) || string.IsNullOrWhiteSpace(request.DeviceName))
                return BadRequest("Device ID and Device Name are required");

            // Check if device is already registered
            var existingDevice = await _ctx.DeviceRegistrations
                .FirstOrDefaultAsync(d => d.DeviceId == request.DeviceId);

            if (existingDevice != null)
                return BadRequest("Device is already registered to a user");

            var deviceRegistration = new DeviceRegistration
            {
                UserId = userId.Value,
                DeviceId = request.DeviceId.Trim().ToUpper(),
                DeviceName = request.DeviceName.Trim(),
                RegisteredAt = DateTime.UtcNow,
                IsActive = true
            };

            _ctx.DeviceRegistrations.Add(deviceRegistration);
            await _ctx.SaveChangesAsync();

            return CreatedAtAction(nameof(GetUserDevices), new { }, deviceRegistration);
        }

        // PUT: api/DeviceRegistration/{id}
        [HttpPut("{id}")]
        public async Task<IActionResult> UpdateDevice(int id, [FromBody] UpdateDeviceRequest request)
        {
            var userId = GetCurrentUserId();
            if (userId == null) return Unauthorized();

            var device = await _ctx.DeviceRegistrations
                .FirstOrDefaultAsync(d => d.DeviceRegistrationId == id && d.UserId == userId.Value);

            if (device == null) return NotFound();

            if (!string.IsNullOrWhiteSpace(request.DeviceName))
                device.DeviceName = request.DeviceName.Trim();

            device.IsActive = request.IsActive;

            await _ctx.SaveChangesAsync();
            return NoContent();
        }

        // DELETE: api/DeviceRegistration/{id}
        [HttpDelete("{id}")]
        public async Task<IActionResult> UnregisterDevice(int id)
        {
            var userId = GetCurrentUserId();
            if (userId == null) return Unauthorized();

            var device = await _ctx.DeviceRegistrations
                .FirstOrDefaultAsync(d => d.DeviceRegistrationId == id && d.UserId == userId.Value);

            if (device == null) return NotFound();

            _ctx.DeviceRegistrations.Remove(device);
            await _ctx.SaveChangesAsync();

            return NoContent();
        }

        private int? GetCurrentUserId()
        {
            var userIdClaim = User.FindFirst(ClaimTypes.NameIdentifier);
            return userIdClaim != null ? int.Parse(userIdClaim.Value) : null;
        }
    }

    public class RegisterDeviceRequest
    {
        public string DeviceId { get; set; } = string.Empty;
        public string DeviceName { get; set; } = string.Empty;
    }

    public class UpdateDeviceRequest
    {
        public string? DeviceName { get; set; }
        public bool IsActive { get; set; } = true;
    }
}