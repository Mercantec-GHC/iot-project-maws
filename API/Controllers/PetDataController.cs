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
    public class PetDataController : ControllerBase
    {
        private readonly AppDBContext _ctx;
        public PetDataController(AppDBContext ctx) => _ctx = ctx;

        // GET: api/PetData
        [HttpGet]
        [Authorize]
        public async Task<ActionResult<IEnumerable<PetData>>> GetAll()
        {
            var userId = GetCurrentUserId();
            if (userId == null) return Unauthorized();

            return Ok(await _ctx.PetData
                .Where(p => p.UserId == userId.Value)
                .OrderByDescending(p => p.Timestamp)
                .ToListAsync());
        }

        // GET: api/PetData/latest
        [HttpGet("latest")]
        [Authorize]
        public async Task<ActionResult<PetData>> GetLatest()
        {
            var userId = GetCurrentUserId();
            if (userId == null) return Unauthorized();

            var latest = await _ctx.PetData
                .Where(p => p.UserId == userId.Value)
                .OrderByDescending(p => p.Timestamp)
                .FirstOrDefaultAsync();
            
            if (latest == null) return NotFound();
            return Ok(latest);
        }

        // POST: api/PetData
        [HttpPost]
        [AllowAnonymous]
        public async Task<ActionResult> Post([FromBody] ArduinoPetData data)
        {
            if (data == null || string.IsNullOrEmpty(data.DeviceId)) 
                return BadRequest("Device ID required");

            // Find device registration
            var deviceRegistration = await _ctx.DeviceRegistrations
                .FirstOrDefaultAsync(d => d.DeviceId == data.DeviceId.ToUpper() && d.IsActive);
            
            if (deviceRegistration == null) 
                return BadRequest("Device not registered or inactive");

            var petData = new PetData
            {
                Hunger = data.Hunger,
                Happiness = data.Happiness,
                Tiredness = data.Tiredness,
                UserId = deviceRegistration.UserId,
                DeviceId = data.DeviceId,
                Timestamp = DateTime.UtcNow
            };

            _ctx.PetData.Add(petData);
            
            // Update last data received timestamp
            deviceRegistration.LastDataReceived = DateTime.UtcNow;
            
            await _ctx.SaveChangesAsync();
            return Ok(new { message = "Data received successfully" });
        }

        // GET: api/PetData/device/{deviceId}
        [HttpGet("device/{deviceId}")]
        [Authorize]
        public async Task<ActionResult<IEnumerable<PetData>>> GetByDeviceId(string deviceId)
        {
            var userId = GetCurrentUserId();
            if (userId == null) return Unauthorized();

            // Verify user owns this device
            var deviceRegistration = await _ctx.DeviceRegistrations
                .FirstOrDefaultAsync(d => d.DeviceId == deviceId.ToUpper() && d.UserId == userId.Value);
            
            if (deviceRegistration == null) return NotFound("Device not found or not owned by user");

            var data = await _ctx.PetData
                .Where(p => p.DeviceId == deviceId && p.UserId == userId.Value)
                .OrderByDescending(p => p.Timestamp)
                .Take(100) // Limit to last 100 records
                .ToListAsync();

            return Ok(data);
        }

        private int? GetCurrentUserId()
        {
            var userIdClaim = User.FindFirst(ClaimTypes.NameIdentifier);
            return userIdClaim != null ? int.Parse(userIdClaim.Value) : null;
        }

        private bool IsAdmin()
        {
            return User.IsInRole("Administrator");
        }
    }

    public class ArduinoPetData
    {
        public string DeviceId { get; set; } = string.Empty;
        public int Hunger { get; set; }
        public int Happiness { get; set; }
        public int Tiredness { get; set; }
    }
}