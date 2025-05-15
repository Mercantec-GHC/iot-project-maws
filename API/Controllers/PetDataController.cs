using API.Data;
using DomainModels;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;

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
        public async Task<ActionResult<IEnumerable<PetData>>> GetAll()
        {
            return Ok(await _ctx.PetData.OrderByDescending(p => p.Timestamp).ToListAsync());
        }

        // GET: api/PetData/latest
        [HttpGet("latest")]
        public async Task<ActionResult<PetData>> GetLatest()
        {
            var latest = await _ctx.PetData.OrderByDescending(p => p.Timestamp).FirstOrDefaultAsync();
            if (latest == null) return NotFound();
            return Ok(latest);
        }

        // POST: api/PetData
        [HttpPost]
        public async Task<ActionResult> Post([FromBody] PetData data)
        {
            if (data == null) return BadRequest();
            data.Timestamp = DateTime.UtcNow;
            _ctx.PetData.Add(data);
            await _ctx.SaveChangesAsync();
            return CreatedAtAction(nameof(GetLatest), new { }, data);
        }
    }
}
