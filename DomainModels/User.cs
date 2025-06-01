using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.ComponentModel.DataAnnotations.Schema;

namespace DomainModels
{
    public class User
    {
        [Column("userid")]
        public int UserId { get; set; }

        [Required]
        [MaxLength(255)]
        [Column("fullname")]
        public string FullName { get; set; } = string.Empty;

        [Required]
        [EmailAddress]
        [MaxLength(255)]
        [Column("email")]
        public string Email { get; set; } = string.Empty;

        [Required]
        [MaxLength(255)]
        [Column("password")]
        public string Password { get; set; } = string.Empty;

        [Column("administrator")]
        public bool Administrator { get; set; } = false;

        [MaxLength(20)]
        [Column("phonenr")]
        public string? PhoneNr { get; set; }

        // Navigation properties
        public virtual ICollection<PetData> PetData { get; set; } = new List<PetData>();
        public virtual ICollection<DeviceRegistration> DeviceRegistrations { get; set; } = new List<DeviceRegistration>();
    }
}