using DomainModels;
using Microsoft.EntityFrameworkCore;
using Microsoft.EntityFrameworkCore.ChangeTracking;

namespace API.Data
{
    public class AppDBContext : DbContext
    {
        public AppDBContext(DbContextOptions<AppDBContext> dbContextOptions) : base(dbContextOptions)
        { }

        public DbSet<User> Users { get; set; }
        public DbSet<Feedback> Feedbacks { get; set; }
        public DbSet<PetData> PetData { get; set; }

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

                // Feedback configuration
                modelBuilder.Entity<Feedback>(entity =>
                {
                    entity.ToTable("Feedbacks");
                    entity.HasKey(f => f.FeedBackId);
                    entity.Property(f => f.FeedbackText).IsRequired();
                });

                modelBuilder.Entity<PetData>(entity =>
                {
                    entity.ToTable("PetData");
                    entity.HasKey(p => p.PetDataId);
                    entity.Property(p => p.Timestamp).IsRequired();
                });
            });

        }
    }
}