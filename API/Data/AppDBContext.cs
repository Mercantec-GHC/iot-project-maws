using DomainModels;
using Microsoft.EntityFrameworkCore;

namespace API.Data
{
    public class AppDBContext : DbContext
    {
        public AppDBContext(DbContextOptions<AppDBContext> dbContextOptions) : base(dbContextOptions)
        { }

        public DbSet<User> Users { get; set; }
        public DbSet<Feedback> Feedbacks { get; set; }
        public DbSet<PetData> PetData { get; set; }
        public DbSet<DeviceRegistration> DeviceRegistrations { get; set; }

        protected override void OnModelCreating(ModelBuilder modelBuilder)
        {
            // User configuration
            modelBuilder.Entity<User>(entity =>
            {
                entity.ToTable("Users");
                entity.HasKey(u => u.UserId);
                entity.Property(u => u.Email).IsRequired().HasMaxLength(255);
                entity.HasIndex(u => u.Email).IsUnique();
                entity.Property(u => u.FullName).IsRequired().HasMaxLength(255);
                entity.Property(u => u.Password).IsRequired().HasMaxLength(255);
                entity.Property(u => u.PhoneNr).HasMaxLength(20);
            });

            // Feedback configuration
            modelBuilder.Entity<Feedback>(entity =>
            {
                entity.ToTable("Feedbacks");
                entity.HasKey(f => f.FeedBackId);
                entity.Property(f => f.FeedbackText).IsRequired();
            });

            // PetData configuration
            modelBuilder.Entity<PetData>(entity =>
            {
                entity.ToTable("PetData");
                entity.HasKey(p => p.PetDataId);
                entity.Property(p => p.Timestamp).IsRequired();
                entity.Property(p => p.UserId).IsRequired();
                entity.Property(p => p.DeviceId).HasMaxLength(50);
                
                // Configure relationship with User
                entity.HasOne(p => p.User)
                      .WithMany(u => u.PetData)
                      .HasForeignKey(p => p.UserId)
                      .OnDelete(DeleteBehavior.Cascade);
            });

            // DeviceRegistration configuration
            modelBuilder.Entity<DeviceRegistration>(entity =>
            {
                entity.ToTable("DeviceRegistrations");
                entity.HasKey(d => d.DeviceRegistrationId);
                entity.Property(d => d.DeviceId).IsRequired().HasMaxLength(50);
                entity.Property(d => d.DeviceName).IsRequired().HasMaxLength(100);
                entity.Property(d => d.UserId).IsRequired();
                
                // Unique constraint: One device can only be registered to one user
                entity.HasIndex(d => d.DeviceId).IsUnique();
                
                // Configure relationship with User
                entity.HasOne(d => d.User)
                      .WithMany(u => u.DeviceRegistrations)
                      .HasForeignKey(d => d.UserId)
                      .OnDelete(DeleteBehavior.Cascade);
            });
        }
    }
}