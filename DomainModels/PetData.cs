using System;
using System.ComponentModel.DataAnnotations;

namespace DomainModels
{
    public class PetData
    {
        [Key]
        public int PetDataId { get; set; }
        
        [Required]
        public int Hunger { get; set; }
        
        [Required]
        public int Happiness { get; set; }
        
        [Required]
        public int Tiredness { get; set; }
        
        public DateTime Timestamp { get; set; } = DateTime.UtcNow;
        
        [Required]
        public int UserId { get; set; }
        
        [MaxLength(50)]
        public string? DeviceId { get; set; }
        
        // Navigation properties
        public virtual User User { get; set; } = null!;
    }
}