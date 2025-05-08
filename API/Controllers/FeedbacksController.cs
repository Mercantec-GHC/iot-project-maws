using API.Data;
using DomainModels;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;

namespace API.Controllers
{
	[ApiController]
	[Route("api/[controller]")]
	public class FeedbacksController : ControllerBase
	{
		private readonly AppDBContext _DBContext;

		public FeedbacksController(AppDBContext DBContext)
		{
			_DBContext = DBContext;
		}

		// Get all Feedbacks
		[HttpGet]
		public async Task<ActionResult<IEnumerable<Feedback>>> GetAllFeedbacks()
		{
			var feedback = await _DBContext.Feedbacks.ToListAsync();
			if (feedback == null)
			{
				return BadRequest();
			}
			return Ok(feedback);
		}

		// Post a Feedback
		[HttpPost]
		public async Task<ActionResult<Feedback>> CreateFeedback(Feedback feedback)
		{
            if (feedback.UserId == 0)
            {
                return Unauthorized("User must be logged in to submit feedback.");
            }

            try
			{
				_DBContext.Feedbacks.Add(feedback);
				await _DBContext.SaveChangesAsync();
				return Ok(StatusCode(200));
			}
			catch (Exception ex)
			{
				return NotFound();
			}
		}

        // Delete a Feedback
        [HttpDelete("{id}")]
        public async Task<IActionResult> DeleteFeedback(int id)
        {
            var feedback = await _DBContext.Feedbacks.FindAsync(id);
            if (feedback == null)
            {
                return NotFound();
            }

            _DBContext.Feedbacks.Remove(feedback);
            await _DBContext.SaveChangesAsync();

            return NoContent();
        }
    }
}