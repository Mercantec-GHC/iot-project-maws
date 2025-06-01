using System;
using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;

namespace DomainModels
{
    public class DeviceRegistration
    {
        [Key]
        public int DeviceRegistrationId { get; set; }
        
        [Required]
        public int UserId { get; set; }
        
        [Required]
        [MaxLength(50)]
        public string DeviceId { get; set; } = string.Empty; // Arduino MAC address or unique ID
        
        [Required]
        [MaxLength(100)]
        public string DeviceName { get; set; } = string.Empty; // User-friendly name
        
        public DateTime RegisteredAt { get; set; } = DateTime.UtcNow;
        
        public bool IsActive { get; set; } = true;
        
        public DateTime? LastDataReceived { get; set; }
        
        // Navigation property
        public virtual User User { get; set; } = null!;
    }
}